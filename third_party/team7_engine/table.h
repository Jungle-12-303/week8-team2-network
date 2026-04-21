#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

#include "bptree.h"

#define RECORD_NAME_SIZE 64

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

typedef struct Table {
    int next_id;
    Record **rows;
    size_t size;
    size_t capacity;
    BPTree *pk_index;
} Table;

Table *table_create(void);
void table_destroy(Table *table);
Record *table_insert(Table *table, const char *name, int age);
Record *table_find_by_id(Table *table, int id);
Record *table_scan_by_id(Table *table, int id);
Record *table_find_by_name(Table *table, const char *name);
Record *table_find_by_age(Table *table, int age);
int table_collect_all(Table *table, Record ***records, size_t *count);
int table_find_by_name_matches(Table *table, const char *name,
    Record ***records, size_t *count);
int table_find_by_id_condition(Table *table, TableComparison comparison,
    int id, Record ***records, size_t *count);
int table_find_by_age_condition(Table *table, TableComparison comparison,
    int age, Record ***records, size_t *count);
void table_print_record(const Record *record);
size_t table_print_records(Record *const *records, size_t row_count);
size_t table_print_all(const Table *table);

#endif /* TABLE_H */
