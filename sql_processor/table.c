#include "table.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fair_rwlock_init(FairRWLock *lock) {
    if (lock == NULL) {
        return 0;
    }

    memset(lock, 0, sizeof(*lock));

    if (pthread_mutex_init(&lock->mutex, NULL) != 0) {
        return 0;
    }

    if (pthread_cond_init(&lock->readers_ok, NULL) != 0) {
        pthread_mutex_destroy(&lock->mutex);
        return 0;
    }

    if (pthread_cond_init(&lock->writers_ok, NULL) != 0) {
        pthread_cond_destroy(&lock->readers_ok);
        pthread_mutex_destroy(&lock->mutex);
        return 0;
    }

    lock->initialized = 1;
    return 1;
}

static void fair_rwlock_destroy(FairRWLock *lock) {
    if (lock == NULL || !lock->initialized) {
        return;
    }

    pthread_cond_destroy(&lock->writers_ok);
    pthread_cond_destroy(&lock->readers_ok);
    pthread_mutex_destroy(&lock->mutex);
    lock->initialized = 0;
}

static int fair_rwlock_rdlock(FairRWLock *lock) {
    if (lock == NULL) {
        return 0;
    }

    if (pthread_mutex_lock(&lock->mutex) != 0) {
        return 0;
    }

    while (lock->active_writer || lock->waiting_writers > 0) {
        if (pthread_cond_wait(&lock->readers_ok, &lock->mutex) != 0) {
            pthread_mutex_unlock(&lock->mutex);
            return 0;
        }
    }

    lock->active_readers++;
    pthread_mutex_unlock(&lock->mutex);
    return 1;
}

static int fair_rwlock_wrlock(FairRWLock *lock) {
    if (lock == NULL) {
        return 0;
    }

    if (pthread_mutex_lock(&lock->mutex) != 0) {
        return 0;
    }

    lock->waiting_writers++;
    while (lock->active_writer || lock->active_readers > 0) {
        if (pthread_cond_wait(&lock->writers_ok, &lock->mutex) != 0) {
            lock->waiting_writers--;
            pthread_mutex_unlock(&lock->mutex);
            return 0;
        }
    }

    lock->waiting_writers--;
    lock->active_writer = 1;
    pthread_mutex_unlock(&lock->mutex);
    return 1;
}

static void fair_rwlock_unlock_read(FairRWLock *lock) {
    if (lock == NULL || pthread_mutex_lock(&lock->mutex) != 0) {
        return;
    }

    if (lock->active_readers > 0) {
        lock->active_readers--;
    }

    if (lock->active_readers == 0) {
        if (lock->waiting_writers > 0) {
            pthread_cond_signal(&lock->writers_ok);
        } else {
            pthread_cond_broadcast(&lock->readers_ok);
        }
    }

    pthread_mutex_unlock(&lock->mutex);
}

static void fair_rwlock_unlock_write(FairRWLock *lock) {
    if (lock == NULL || pthread_mutex_lock(&lock->mutex) != 0) {
        return;
    }

    lock->active_writer = 0;
    if (lock->waiting_writers > 0) {
        pthread_cond_signal(&lock->writers_ok);
    } else {
        pthread_cond_broadcast(&lock->readers_ok);
    }

    pthread_mutex_unlock(&lock->mutex);
}

/* Frees all bucket-owned records and the B+ tree indexes. */
void table_destroy(Table *table);

/* Returns the bucket index for a given row ID. */
static size_t table_bucket_index_for_id(int id) {
    int bucket = id % TABLE_BUCKET_COUNT;

    if (bucket < 0) {
        bucket += TABLE_BUCKET_COUNT;
    }

    return (size_t)bucket;
}

/* Grows the record pointer array when it becomes full. */
static int table_ensure_capacity(TableBucket *bucket) {
    size_t new_capacity;
    Record **new_rows;

    if (bucket->size < bucket->capacity) {
        return 1;
    }

    new_capacity = (bucket->capacity == 0) ? 8 : bucket->capacity * 2;
    new_rows = (Record **)realloc(bucket->rows, new_capacity * sizeof(Record *));
    if (new_rows == NULL) {
        return 0;
    }

    bucket->rows = new_rows;
    bucket->capacity = new_capacity;
    return 1;
}

/* Appends one record pointer to a growable result array. */
static int table_append_record(Record ***records, size_t *count, size_t *capacity, Record *record) {
    Record **new_records;
    size_t new_capacity;

    if (*count < *capacity) {
        (*records)[*count] = record;
        (*count)++;
        return 1;
    }

    new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
    new_records = (Record **)realloc(*records, new_capacity * sizeof(Record *));
    if (new_records == NULL) {
        return 0;
    }

    *records = new_records;
    *capacity = new_capacity;
    (*records)[*count] = record;
    (*count)++;
    return 1;
}

/* Sorts record pointers by their row ID. */
static int table_compare_records_by_id(const void *left, const void *right) {
    const Record *const *lhs = (const Record *const *)left;
    const Record *const *rhs = (const Record *const *)right;

    if ((*lhs)->id < (*rhs)->id) {
        return -1;
    }
    if ((*lhs)->id > (*rhs)->id) {
        return 1;
    }
    return 0;
}

/* Sorts a record pointer array in ascending ID order. */
static void table_sort_records_by_id(Record **records, size_t count) {
    if (records == NULL || count < 2) {
        return;
    }

    qsort(records, count, sizeof(Record *), table_compare_records_by_id);
}

/* Evaluates an integer comparison used by WHERE clauses. */
static int table_compare_int(int left, TableComparison comparison, int right) {
    switch (comparison) {
        case TABLE_COMPARISON_EQ:
            return left == right;
        case TABLE_COMPARISON_LT:
            return left < right;
        case TABLE_COMPARISON_LE:
            return left <= right;
        case TABLE_COMPARISON_GT:
            return left > right;
        case TABLE_COMPARISON_GE:
            return left >= right;
    }

    return 0;
}

/* Frees a dynamically allocated record pointer array. */
static void table_free_record_array(Record ***records, size_t *count) {
    if (records == NULL || count == NULL) {
        return;
    }

    free(*records);
    *records = NULL;
    *count = 0;
}

/* Collects rows from one bucket while the caller already holds the lock. */
static int table_collect_bucket_rows_locked(TableBucket *bucket, Record ***records, size_t *count, size_t *capacity) {
    size_t index;

    for (index = 0; index < bucket->size; index++) {
        if (!table_append_record(records, count, capacity, bucket->rows[index])) {
            table_free_record_array(records, count);
            return 0;
        }
    }

    return 1;
}

/* Collects every record from all buckets and sorts the final array by ID. */
static int table_collect_all_sorted(Table *table, Record ***records, size_t *count) {
    size_t index;
    size_t capacity = 0;

    if (table == NULL || records == NULL || count == NULL) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    for (index = 0; index < TABLE_BUCKET_COUNT; index++) {
        TableBucket *bucket = &table->buckets[index];

        if (!fair_rwlock_rdlock(&bucket->lock)) {
            table_free_record_array(records, count);
            return 0;
        }

        if (!table_collect_bucket_rows_locked(bucket, records, count, &capacity)) {
            fair_rwlock_unlock_read(&bucket->lock);
            table_free_record_array(records, count);
            return 0;
        }

        fair_rwlock_unlock_read(&bucket->lock);
    }

    table_sort_records_by_id(*records, *count);
    return 1;
}

/* Creates the in-memory users table with hash-based buckets. */
Table *table_create(void) {
    Table *table;
    size_t index;

    table = (Table *)calloc(1, sizeof(Table));
    if (table == NULL) {
        return NULL;
    }

    table->next_id = 1;

    if (pthread_mutex_init(&table->next_id_lock, NULL) != 0) {
        free(table);
        return NULL;
    }
    table->next_id_lock_initialized = 1;

    for (index = 0; index < TABLE_BUCKET_COUNT; index++) {
        TableBucket *bucket = &table->buckets[index];

        if (!fair_rwlock_init(&bucket->lock)) {
            table_destroy(table);
            return NULL;
        }

        table->initialized_buckets = index + 1;
        bucket->pk_index = bptree_create();
        if (bucket->pk_index == NULL) {
            table_destroy(table);
            return NULL;
        }
    }

    table->initialized_buckets = TABLE_BUCKET_COUNT;
    return table;
}

/* Frees all bucket-owned records and the B+ tree indexes. */
void table_destroy(Table *table) {
    size_t index;

    if (table == NULL) {
        return;
    }

    for (index = 0; index < table->initialized_buckets; index++) {
        TableBucket *bucket = &table->buckets[index];
        size_t row_index;

        for (row_index = 0; row_index < bucket->size; row_index++) {
            free(bucket->rows[row_index]);
        }
        free(bucket->rows);
        bucket->rows = NULL;
        bucket->size = 0;
        bucket->capacity = 0;

        bptree_destroy(bucket->pk_index);
        bucket->pk_index = NULL;

        fair_rwlock_destroy(&bucket->lock);
    }

    if (table->next_id_lock_initialized) {
        pthread_mutex_destroy(&table->next_id_lock);
    }

    free(table);
}

/* Inserts one record and returns the stored record pointer. */
Record *table_insert(Table *table, const char *name, int age) {
    TableBucket *bucket;
    Record *record;
    int record_id;

    if (table == NULL || name == NULL) {
        return NULL;
    }

    if (pthread_mutex_lock(&table->next_id_lock) != 0) {
        return NULL;
    }

    record_id = table->next_id++;
    pthread_mutex_unlock(&table->next_id_lock);

    bucket = &table->buckets[table_bucket_index_for_id(record_id)];

    if (!fair_rwlock_wrlock(&bucket->lock)) {
        return NULL;
    }

    if (!table_ensure_capacity(bucket)) {
        fair_rwlock_unlock_write(&bucket->lock);
        return NULL;
    }

    record = (Record *)calloc(1, sizeof(Record));
    if (record == NULL) {
        fair_rwlock_unlock_write(&bucket->lock);
        return NULL;
    }

    record->id = record_id;
    strncpy(record->name, name, RECORD_NAME_SIZE - 1);
    record->name[RECORD_NAME_SIZE - 1] = '\0';
    record->age = age;

    if (!bptree_insert(bucket->pk_index, record->id, record)) {
        free(record);
        fair_rwlock_unlock_write(&bucket->lock);
        return NULL;
    }

    bucket->rows[bucket->size] = record;
    bucket->size++;
    fair_rwlock_unlock_write(&bucket->lock);
    return record;
}

/* Looks up a record by ID using the bucket-local B+ tree. */
Record *table_find_by_id(Table *table, int id) {
    TableBucket *bucket;
    Record *record;

    if (table == NULL) {
        return NULL;
    }

    bucket = &table->buckets[table_bucket_index_for_id(id)];
    if (!fair_rwlock_rdlock(&bucket->lock)) {
        return NULL;
    }

    record = (Record *)bptree_search(bucket->pk_index, id);
    fair_rwlock_unlock_read(&bucket->lock);
    return record;
}

/* Looks up a record by ID using a linear scan across all buckets. */
Record *table_scan_by_id(Table *table, int id) {
    size_t bucket_index;
    size_t row_index;

    if (table == NULL) {
        return NULL;
    }

    for (bucket_index = 0; bucket_index < TABLE_BUCKET_COUNT; bucket_index++) {
        TableBucket *bucket = &table->buckets[bucket_index];

        if (!fair_rwlock_rdlock(&bucket->lock)) {
            return NULL;
        }

        for (row_index = 0; row_index < bucket->size; row_index++) {
            if (bucket->rows[row_index]->id == id) {
                Record *record = bucket->rows[row_index];
                fair_rwlock_unlock_read(&bucket->lock);
                return record;
            }
        }

        fair_rwlock_unlock_read(&bucket->lock);
    }

    return NULL;
}

/* Copies every stored record pointer into a result array. */
int table_collect_all(Table *table, Record ***records, size_t *count) {
    return table_collect_all_sorted(table, records, count);
}

/* Looks up the first record with a matching name using bucket scans. */
Record *table_find_by_name(Table *table, const char *name) {
    Record **records = NULL;
    size_t count = 0;
    Record *record = NULL;

    if (table == NULL || name == NULL) {
        return NULL;
    }

    if (!table_find_by_name_matches(table, name, &records, &count)) {
        return NULL;
    }

    if (count > 0) {
        record = records[0];
    }

    free(records);
    return record;
}

/* Looks up the first record with a matching age using bucket scans. */
Record *table_find_by_age(Table *table, int age) {
    Record **records = NULL;
    size_t count = 0;
    Record *record = NULL;

    if (table == NULL) {
        return NULL;
    }

    if (!table_find_by_age_condition(table, TABLE_COMPARISON_EQ, age, &records, &count)) {
        return NULL;
    }

    if (count > 0) {
        record = records[0];
    }

    free(records);
    return record;
}

/* Collects every record with a matching name using bucket scans. */
int table_find_by_name_matches(Table *table, const char *name, Record ***records, size_t *count) {
    Record **all_records = NULL;
    size_t all_count = 0;
    size_t capacity = 0;
    size_t index;

    if (table == NULL || name == NULL || records == NULL || count == NULL) {
        return 0;
    }

    if (!table_collect_all_sorted(table, &all_records, &all_count)) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    for (index = 0; index < all_count; index++) {
        if (strcmp(all_records[index]->name, name) == 0) {
            if (!table_append_record(records, count, &capacity, all_records[index])) {
                free(all_records);
                table_free_record_array(records, count);
                return 0;
            }
        }
    }

    free(all_records);
    return 1;
}

/* Collects every record whose ID satisfies the given comparison. */
int table_find_by_id_condition(Table *table, TableComparison comparison, int id, Record ***records, size_t *count) {
    Record **all_records = NULL;
    size_t all_count = 0;
    size_t capacity = 0;
    size_t index;

    if (table == NULL || records == NULL || count == NULL) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    if (comparison == TABLE_COMPARISON_EQ) {
        Record *record = table_find_by_id(table, id);

        if (record == NULL) {
            return 1;
        }

        return table_append_record(records, count, &capacity, record);
    }

    if (!table_collect_all_sorted(table, &all_records, &all_count)) {
        return 0;
    }

    for (index = 0; index < all_count; index++) {
        if (table_compare_int(all_records[index]->id, comparison, id)) {
            if (!table_append_record(records, count, &capacity, all_records[index])) {
                free(all_records);
                table_free_record_array(records, count);
                return 0;
            }
        }
    }

    free(all_records);
    return 1;
}

/* Collects every record whose age satisfies the given comparison. */
int table_find_by_age_condition(Table *table, TableComparison comparison, int age, Record ***records, size_t *count) {
    Record **all_records = NULL;
    size_t all_count = 0;
    size_t capacity = 0;
    size_t index;

    if (table == NULL || records == NULL || count == NULL) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    if (!table_collect_all_sorted(table, &all_records, &all_count)) {
        return 0;
    }

    for (index = 0; index < all_count; index++) {
        if (table_compare_int(all_records[index]->age, comparison, age)) {
            if (!table_append_record(records, count, &capacity, all_records[index])) {
                free(all_records);
                table_free_record_array(records, count);
                return 0;
            }
        }
    }

    free(all_records);
    return 1;
}

/* Prints a single record in a compact presentation-friendly format. */
void table_print_record(const Record *record) {
    if (record == NULL) {
        return;
    }

    printf("id=%d, name='%s', age=%d\n", record->id, record->name, record->age);
}

/* Prints an already collected record pointer array. */
size_t table_print_records(Record *const *records, size_t row_count) {
    size_t index;

    if (records == NULL) {
        return 0;
    }

    for (index = 0; index < row_count; index++) {
        table_print_record(records[index]);
    }

    return row_count;
}

/* Prints all records in sorted-by-ID order and returns the printed row count. */
size_t table_print_all(const Table *table) {
    Record **records = NULL;
    size_t count = 0;
    size_t printed;

    if (table == NULL) {
        return 0;
    }

    if (!table_collect_all_sorted((Table *)table, &records, &count)) {
        return 0;
    }

    printed = table_print_records(records, count);
    free(records);
    return printed;
}
