#include "table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int table_ensure_capacity(Table *table)
{
    size_t new_capacity;
    Record **new_rows;

    if (table->size < table->capacity) {
        return 1;
    }

    new_capacity = (table->capacity == 0) ? 8 : table->capacity * 2;
    new_rows = (Record **)realloc(table->rows,
        new_capacity * sizeof(Record *));
    if (new_rows == NULL) {
        return 0;
    }

    table->rows = new_rows;
    table->capacity = new_capacity;
    return 1;
}

static int table_append_record(Record ***records, size_t *count,
    size_t *capacity, Record *record)
{
    Record **new_records;
    size_t new_capacity;

    if (*count < *capacity) {
        (*records)[*count] = record;
        (*count)++;
        return 1;
    }

    new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
    new_records = (Record **)realloc(*records,
        new_capacity * sizeof(Record *));
    if (new_records == NULL) {
        return 0;
    }

    *records = new_records;
    *capacity = new_capacity;
    (*records)[*count] = record;
    (*count)++;
    return 1;
}

static int table_copy_records(Record *const *source, size_t source_count,
    Record ***records, size_t *count)
{
    Record **copy;

    *records = NULL;
    *count = 0;

    if (source_count == 0) {
        return 1;
    }

    copy = (Record **)malloc(source_count * sizeof(Record *));
    if (copy == NULL) {
        return 0;
    }

    memcpy(copy, source, source_count * sizeof(Record *));
    *records = copy;
    *count = source_count;
    return 1;
}

static int table_compare_int(int left, TableComparison comparison, int right)
{
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

static int table_find_insert_index(const int *keys, int num_keys, int key)
{
    int index = 0;

    while (index < num_keys && keys[index] < key) {
        index++;
    }

    return index;
}

static BPTreeNode *table_find_leftmost_leaf(Table *table)
{
    BPTreeNode *node;

    if (table == NULL || table->pk_index == NULL ||
        table->pk_index->root == NULL) {
        return NULL;
    }

    node = table->pk_index->root;
    while (!node->is_leaf) {
        node = node->children[0];
    }

    return node;
}

static BPTreeNode *table_find_id_leaf(Table *table, int id)
{
    BPTreeNode *node;
    int child_index;

    if (table == NULL || table->pk_index == NULL ||
        table->pk_index->root == NULL) {
        return NULL;
    }

    node = table->pk_index->root;
    while (!node->is_leaf) {
        child_index = 0;

        while (child_index < node->num_keys &&
            id >= node->keys[child_index]) {
            child_index++;
        }

        node = node->children[child_index];
    }

    return node;
}

static int table_collect_leaf_chain(BPTreeNode *leaf, int start_index,
    Record ***records, size_t *count)
{
    size_t capacity = 0;
    int index;

    *records = NULL;
    *count = 0;

    while (leaf != NULL) {
        for (index = start_index; index < leaf->num_keys; index++) {
            if (!table_append_record(records, count, &capacity,
                    (Record *)leaf->values[index])) {
                free(*records);
                *records = NULL;
                *count = 0;
                return 0;
            }
        }

        leaf = leaf->next;
        start_index = 0;
    }

    return 1;
}

Table *table_create(void)
{
    Table *table;

    table = (Table *)calloc(1, sizeof(Table));
    if (table == NULL) {
        return NULL;
    }

    table->next_id = 1;
    table->pk_index = bptree_create();
    if (table->pk_index == NULL) {
        free(table);
        return NULL;
    }

    return table;
}

void table_destroy(Table *table)
{
    size_t index;

    if (table == NULL) {
        return;
    }

    for (index = 0; index < table->size; index++) {
        free(table->rows[index]);
    }

    free(table->rows);
    bptree_destroy(table->pk_index);
    free(table);
}

Record *table_insert(Table *table, const char *name, int age)
{
    Record *record;

    if (table == NULL || name == NULL) {
        return NULL;
    }

    if (!table_ensure_capacity(table)) {
        return NULL;
    }

    record = (Record *)calloc(1, sizeof(Record));
    if (record == NULL) {
        return NULL;
    }

    record->id = table->next_id++;
    strncpy(record->name, name, RECORD_NAME_SIZE - 1);
    record->name[RECORD_NAME_SIZE - 1] = '\0';
    record->age = age;

    table->rows[table->size] = record;

    if (!bptree_insert(table->pk_index, record->id, record)) {
        free(record);
        return NULL;
    }

    table->size++;
    return record;
}

Record *table_find_by_id(Table *table, int id)
{
    if (table == NULL) {
        return NULL;
    }

    return (Record *)bptree_search(table->pk_index, id);
}

Record *table_scan_by_id(Table *table, int id)
{
    size_t index;

    if (table == NULL) {
        return NULL;
    }

    for (index = 0; index < table->size; index++) {
        if (table->rows[index]->id == id) {
            return table->rows[index];
        }
    }

    return NULL;
}

Record *table_find_by_name(Table *table, const char *name)
{
    size_t index;

    if (table == NULL || name == NULL) {
        return NULL;
    }

    for (index = 0; index < table->size; index++) {
        if (strcmp(table->rows[index]->name, name) == 0) {
            return table->rows[index];
        }
    }

    return NULL;
}

Record *table_find_by_age(Table *table, int age)
{
    size_t index;

    if (table == NULL) {
        return NULL;
    }

    for (index = 0; index < table->size; index++) {
        if (table->rows[index]->age == age) {
            return table->rows[index];
        }
    }

    return NULL;
}

int table_collect_all(Table *table, Record ***records, size_t *count)
{
    if (table == NULL || records == NULL || count == NULL) {
        return 0;
    }

    return table_copy_records(table->rows, table->size, records, count);
}

int table_find_by_name_matches(Table *table, const char *name,
    Record ***records, size_t *count)
{
    size_t index;
    size_t capacity = 0;

    if (table == NULL || name == NULL || records == NULL || count == NULL) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    for (index = 0; index < table->size; index++) {
        if (strcmp(table->rows[index]->name, name) == 0) {
            if (!table_append_record(records, count, &capacity,
                    table->rows[index])) {
                free(*records);
                *records = NULL;
                *count = 0;
                return 0;
            }
        }
    }

    return 1;
}

int table_find_by_id_condition(Table *table, TableComparison comparison,
    int id, Record ***records, size_t *count)
{
    BPTreeNode *leaf;
    size_t capacity = 0;
    int index;

    if (table == NULL || records == NULL || count == NULL) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    if (comparison == TABLE_COMPARISON_EQ) {
        Record *record;

        record = table_find_by_id(table, id);
        if (record == NULL) {
            return 1;
        }

        return table_append_record(records, count, &capacity, record);
    }

    if (comparison == TABLE_COMPARISON_GT ||
        comparison == TABLE_COMPARISON_GE) {
        leaf = table_find_id_leaf(table, id);
        if (leaf == NULL) {
            return 1;
        }

        index = table_find_insert_index(leaf->keys, leaf->num_keys, id);
        if (comparison == TABLE_COMPARISON_GT) {
            while (index < leaf->num_keys && leaf->keys[index] == id) {
                index++;
            }
        }

        return table_collect_leaf_chain(leaf, index, records, count);
    }

    leaf = table_find_leftmost_leaf(table);
    while (leaf != NULL) {
        for (index = 0; index < leaf->num_keys; index++) {
            Record *record;

            record = (Record *)leaf->values[index];
            if (table_compare_int(record->id, comparison, id)) {
                if (!table_append_record(records, count, &capacity, record)) {
                    free(*records);
                    *records = NULL;
                    *count = 0;
                    return 0;
                }
            }
        }
        leaf = leaf->next;
    }

    return 1;
}

int table_find_by_age_condition(Table *table, TableComparison comparison,
    int age, Record ***records, size_t *count)
{
    size_t index;
    size_t capacity = 0;

    if (table == NULL || records == NULL || count == NULL) {
        return 0;
    }

    *records = NULL;
    *count = 0;

    for (index = 0; index < table->size; index++) {
        if (table_compare_int(table->rows[index]->age, comparison, age)) {
            if (!table_append_record(records, count, &capacity,
                    table->rows[index])) {
                free(*records);
                *records = NULL;
                *count = 0;
                return 0;
            }
        }
    }

    return 1;
}

void table_print_record(const Record *record)
{
    if (record == NULL) {
        return;
    }

    printf("id=%d name=%s age=%d\n", record->id, record->name, record->age);
}

size_t table_print_records(Record *const *records, size_t row_count)
{
    size_t index;

    if (records == NULL) {
        return 0;
    }

    for (index = 0; index < row_count; index++) {
        table_print_record(records[index]);
    }

    return row_count;
}

size_t table_print_all(const Table *table)
{
    size_t index;

    if (table == NULL) {
        return 0;
    }

    for (index = 0; index < table->size; index++) {
        table_print_record(table->rows[index]);
    }

    return table->size;
}
