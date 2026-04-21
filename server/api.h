#ifndef SERVER_API_H
#define SERVER_API_H

#include <pthread.h>

#include "sql.h"
#include "table.h"

typedef struct ApiResult {
    int http_status;
    char *body;
} ApiResult;

int api_handle_query(Table *table, pthread_mutex_t *db_mutex, const char *sql, ApiResult *result);
void api_result_destroy(ApiResult *result);

#endif
