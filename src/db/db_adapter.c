#include "db_adapter.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../../third_party/team7_engine/sql.h"
#include "../../third_party/team7_engine/table.h"

struct DbHandle {
    Table *table;
    pthread_rwlock_t lock;
};

static void db_result_reset(DbResult *result)
{
    if (result == NULL) {
        return;
    }

    result->status = DB_STATUS_INTERNAL_ERROR;
    result->inserted_id = 0;
    result->rows = NULL;
    result->row_count = 0;
    result->error_message[0] = '\0';
}

static int db_is_readonly_sql(const char *sql)
{
    while (*sql == ' ' || *sql == '\t' || *sql == '\n' || *sql == '\r') {
        sql++;
    }

    return strncasecmp(sql, "SELECT", 6) == 0;
}

static DbStatus db_map_status(SQLStatus status)
{
    switch (status) {
        case SQL_STATUS_OK:
            return DB_STATUS_OK;
        case SQL_STATUS_NOT_FOUND:
            return DB_STATUS_NOT_FOUND;
        case SQL_STATUS_SYNTAX_ERROR:
        case SQL_STATUS_QUERY_ERROR:
            return DB_STATUS_BAD_REQUEST;
        case SQL_STATUS_EXIT:
        case SQL_STATUS_ERROR:
        default:
            return DB_STATUS_INTERNAL_ERROR;
    }
}

static int db_copy_rows(const SQLResult *sql_result, DbResult *out)
{
    size_t index;

    if (sql_result->row_count == 0) {
        return 1;
    }

    out->rows = (DbRow *)calloc(sql_result->row_count, sizeof(DbRow));
    if (out->rows == NULL) {
        return 0;
    }

    for (index = 0; index < sql_result->row_count; index++) {
        out->rows[index].id = sql_result->records[index]->id;
        strncpy(out->rows[index].name, sql_result->records[index]->name,
            sizeof(out->rows[index].name) - 1);
        out->rows[index].age = sql_result->records[index]->age;
    }

    out->row_count = sql_result->row_count;
    return 1;
}

DbHandle *db_init(void)
{
    DbHandle *db;

    db = (DbHandle *)calloc(1, sizeof(DbHandle));
    if (db == NULL) {
        return NULL;
    }

    db->table = table_create();
    if (db->table == NULL) {
        free(db);
        return NULL;
    }

    if (pthread_rwlock_init(&db->lock, NULL) != 0) {
        table_destroy(db->table);
        free(db);
        return NULL;
    }

    return db;
}

void db_shutdown(DbHandle *db)
{
    if (db == NULL) {
        return;
    }

    pthread_rwlock_destroy(&db->lock);
    table_destroy(db->table);
    free(db);
}

int db_execute_sql(DbHandle *db, const char *sql, DbResult *out)
{
    SQLResult sql_result;
    int lock_result;
    int ok = 1;

    if (db == NULL || sql == NULL || out == NULL) {
        return 0;
    }

    db_result_reset(out);

    if (db_is_readonly_sql(sql)) {
        lock_result = pthread_rwlock_rdlock(&db->lock);
    } else {
        lock_result = pthread_rwlock_wrlock(&db->lock);
    }

    if (lock_result != 0) {
        snprintf(out->error_message, sizeof(out->error_message),
            "failed to acquire database lock");
        return 0;
    }

    sql_result = sql_execute(db->table, sql);

    out->status = db_map_status(sql_result.status);
    out->inserted_id = sql_result.inserted_id;

    if (sql_result.error_message[0] != '\0') {
        strncpy(out->error_message, sql_result.error_message,
            sizeof(out->error_message) - 1);
    }

    if (sql_result.action == SQL_ACTION_SELECT_ROWS &&
        (out->status == DB_STATUS_OK || out->status == DB_STATUS_NOT_FOUND)) {
        if (!db_copy_rows(&sql_result, out)) {
            out->status = DB_STATUS_INTERNAL_ERROR;
            snprintf(out->error_message, sizeof(out->error_message),
                "failed to copy SQL result rows");
            ok = 0;
        }
    }

    sql_result_destroy(&sql_result);
    pthread_rwlock_unlock(&db->lock);
    return ok;
}

void db_result_destroy(DbResult *result)
{
    if (result == NULL) {
        return;
    }

    free(result->rows);
    result->rows = NULL;
    result->row_count = 0;
    result->inserted_id = 0;
    result->error_message[0] = '\0';
    result->status = DB_STATUS_INTERNAL_ERROR;
}
