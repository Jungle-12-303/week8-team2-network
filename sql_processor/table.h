#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include <pthread.h>

#include "bptree.h"

#define RECORD_NAME_SIZE 64
#define TABLE_BUCKET_COUNT 16

typedef struct Record {
    int id;
    char name[RECORD_NAME_SIZE];
    int age;
} Record;

typedef enum TableComparison {
    TABLE_COMPARISON_EQ,
    TABLE_COMPARISON_LT,
    TABLE_COMPARISON_LE,
    TABLE_COMPARISON_GT,
    TABLE_COMPARISON_GE
} TableComparison;

typedef struct TableBucket {
    pthread_rwlock_t lock;
    Record **rows;
    size_t size;
    size_t capacity;
    BPTree *pk_index;
} TableBucket;

typedef struct Table {
    int next_id;
    pthread_mutex_t next_id_lock;
    int next_id_lock_initialized;
    size_t initialized_buckets;
    TableBucket buckets[TABLE_BUCKET_COUNT];
} Table;

/* Creates the in-memory users table with hash-based buckets. */
Table *table_create(void);

/* Frees all bucket-owned records, indexes, and locks. */
void table_destroy(Table *table);

/* Inserts one record and returns the stored record pointer. */
Record *table_insert(Table *table, const char *name, int age);

/* Looks up a record by ID using the bucket-local B+ tree index. */
Record *table_find_by_id(Table *table, int id);

/* Looks up a record by ID using a linear scan across all buckets. */
Record *table_scan_by_id(Table *table, int id);

/* Looks up the first record with a matching name using bucket scans. */
Record *table_find_by_name(Table *table, const char *name);

/* Looks up the first record with a matching age using bucket scans. */
Record *table_find_by_age(Table *table, int age);

/* Copies every stored record pointer into a result array. */
int table_collect_all(Table *table, Record ***records, size_t *count);

/* Collects every record with a matching name using bucket scans. */
int table_find_by_name_matches(Table *table, const char *name, Record ***records, size_t *count);

/* Collects every record whose ID satisfies the given comparison. */
int table_find_by_id_condition(Table *table, TableComparison comparison, int id, Record ***records, size_t *count);

/* Collects every record whose age satisfies the given comparison. */
int table_find_by_age_condition(Table *table, TableComparison comparison, int age, Record ***records, size_t *count);

/* Prints a single record in a compact presentation-friendly format. */
void table_print_record(const Record *record);

/* Prints an already collected record pointer array. */
size_t table_print_records(Record *const *records, size_t row_count);

/* Prints all records in sorted-by-ID order and returns the printed row count. */
size_t table_print_all(const Table *table);

#endif
