#ifndef DB_ADAPTER_H
#define DB_ADAPTER_H

#include <stddef.h>

typedef enum DbStatus {
    DB_STATUS_OK,
    DB_STATUS_NOT_FOUND,
    DB_STATUS_BAD_REQUEST,
    DB_STATUS_INTERNAL_ERROR
} DbStatus;

typedef struct DbRow {
    int id;
    char name[64];
    int age;
} DbRow;

typedef struct DbResult {
    DbStatus status;
    int inserted_id;
    DbRow *rows;
    size_t row_count;
    char error_message[256];
} DbResult;

typedef struct DbHandle DbHandle;

DbHandle *db_init(void);
void db_shutdown(DbHandle *db);
int db_execute_sql(DbHandle *db, const char *sql, DbResult *out);
void db_result_destroy(DbResult *result);

#endif /* DB_ADAPTER_H */
