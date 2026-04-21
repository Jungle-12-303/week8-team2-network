#include "api.h"

#include "json_util.h"

#include <stdlib.h>
#include <string.h>

static const char *api_action_name(SQLAction action) {
    switch (action) {
        case SQL_ACTION_INSERT:
            return "insert";
        case SQL_ACTION_SELECT_ROWS:
            return "select";
        case SQL_ACTION_NONE:
        default:
            return "none";
    }
}

static const char *api_status_name(SQLStatus status) {
    switch (status) {
        case SQL_STATUS_SYNTAX_ERROR:
            return "syntax_error";
        case SQL_STATUS_QUERY_ERROR:
            return "query_error";
        case SQL_STATUS_NOT_FOUND:
            return "not_found";
        case SQL_STATUS_ERROR:
            return "internal_error";
        case SQL_STATUS_EXIT:
            return "exit_not_supported";
        case SQL_STATUS_OK:
        default:
            return "ok";
    }
}

static int api_append_record(JsonBuffer *buffer, const Record *record) {
    return json_buffer_appendf(buffer, "{\"id\":%d,\"name\":", record->id) &&
           json_buffer_append_json_string(buffer, record->name) &&
           json_buffer_appendf(buffer, ",\"age\":%d}", record->age);
}

static int api_build_success_response(const SQLResult *sql_result, JsonBuffer *buffer) {
    size_t index;

    if (!json_buffer_append(buffer, "{\"ok\":true,\"action\":")) {
        return 0;
    }

    if (!json_buffer_append_json_string(buffer, api_action_name(sql_result->action))) {
        return 0;
    }

    if (sql_result->action == SQL_ACTION_INSERT) {
        return json_buffer_appendf(
            buffer,
            ",\"inserted_id\":%d,\"row_count\":%zu}",
            sql_result->inserted_id,
            sql_result->row_count
        );
    }

    if (!json_buffer_appendf(buffer, ",\"row_count\":%zu,\"rows\":[", sql_result->row_count)) {
        return 0;
    }

    for (index = 0; index < sql_result->row_count; index++) {
        if (index > 0 && !json_buffer_append(buffer, ",")) {
            return 0;
        }
        if (!api_append_record(buffer, sql_result->records[index])) {
            return 0;
        }
    }

    return json_buffer_append(buffer, "]}");
}

static int api_build_error_response(const SQLResult *sql_result, JsonBuffer *buffer) {
    if (!json_buffer_append(buffer, "{\"ok\":false,\"status\":")) {
        return 0;
    }

    if (!json_buffer_append_json_string(buffer, api_status_name(sql_result->status))) {
        return 0;
    }

    if (!json_buffer_appendf(buffer, ",\"error_code\":%d,\"sql_state\":", sql_result->error_code)) {
        return 0;
    }

    if (!json_buffer_append_json_string(buffer, sql_result->sql_state)) {
        return 0;
    }

    if (!json_buffer_append(buffer, ",\"message\":")) {
        return 0;
    }

    if (!json_buffer_append_json_string(buffer, sql_result->error_message[0] != '\0' ? sql_result->error_message : "Unknown SQL error")) {
        return 0;
    }

    return json_buffer_append(buffer, "}");
}

int api_handle_query(Table *table, pthread_mutex_t *db_mutex, const char *sql, ApiResult *result) {
    SQLResult sql_result;
    JsonBuffer buffer;
    int ok;

    if (result == NULL) {
        return 0;
    }

    result->http_status = 500;
    result->body = NULL;

    if (table == NULL || db_mutex == NULL || sql == NULL) {
        return 0;
    }

    if (pthread_mutex_lock(db_mutex) != 0) {
        return 0;
    }

    sql_result = sql_execute(table, sql);

    if (!json_buffer_init(&buffer, 256)) {
        sql_result_destroy(&sql_result);
        pthread_mutex_unlock(db_mutex);
        return 0;
    }

    if (sql_result.status == SQL_STATUS_OK || sql_result.status == SQL_STATUS_NOT_FOUND) {
        ok = api_build_success_response(&sql_result, &buffer);
        result->http_status = 200;
    } else if (sql_result.status == SQL_STATUS_EXIT) {
        ok = json_buffer_append(
            &buffer,
            "{\"ok\":false,\"status\":\"exit_not_supported\",\"message\":\"EXIT and QUIT are not supported over HTTP\"}"
        );
        result->http_status = 400;
    } else if (sql_result.status == SQL_STATUS_ERROR) {
        ok = json_buffer_append(
            &buffer,
            "{\"ok\":false,\"status\":\"internal_error\",\"message\":\"Internal database error\"}"
        );
        result->http_status = 500;
    } else {
        ok = api_build_error_response(&sql_result, &buffer);
        result->http_status = 200;
    }

    sql_result_destroy(&sql_result);
    pthread_mutex_unlock(db_mutex);

    if (!ok) {
        json_buffer_destroy(&buffer);
        return 0;
    }

    result->body = json_buffer_detach(&buffer);
    json_buffer_destroy(&buffer);
    return result->body != NULL;
}

void api_result_destroy(ApiResult *result) {
    if (result == NULL) {
        return;
    }

    free(result->body);
    result->body = NULL;
    result->http_status = 0;
}
