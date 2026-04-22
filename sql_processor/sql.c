#include "sql.h"

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* Builds a zero-initialized SQL result with the requested defaults. */
static SQLResult sql_make_result(SQLStatus status, SQLAction action) {
    SQLResult result;

    result.status = status;
    result.action = action;
    result.record = NULL;
    result.records = NULL;
    result.inserted_id = 0;
    result.row_count = 0;
    result.error_code = 0;
    result.sql_state[0] = '\0';
    result.error_message[0] = '\0';
    return result;
}

static int sql_is_known_column(const char *column) {
    return strcasecmp(column, "id") == 0 ||
           strcasecmp(column, "name") == 0 ||
           strcasecmp(column, "age") == 0;
}

static void sql_build_near_excerpt(const char *cursor, char *buffer, size_t buffer_size) {
    size_t length = 0;

    while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
        cursor++;
    }

    if (*cursor == '\0') {
        snprintf(buffer, buffer_size, "end of input");
        return;
    }

    while (cursor[length] != '\0' &&
           cursor[length] != '\n' &&
           cursor[length] != '\r' &&
           cursor[length] != ';' &&
           length + 1 < buffer_size) {
        buffer[length] = cursor[length];
        length++;
    }

    while (length > 0 && isspace((unsigned char)buffer[length - 1])) {
        length--;
    }

    buffer[length] = '\0';

    if (length == 0) {
        snprintf(buffer, buffer_size, "end of input");
    }
}

/* Skips whitespace characters in the parser cursor. */
static void sql_skip_spaces(const char **cursor) {
    while (**cursor != '\0' && isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }
}

/* Matches a keyword case-insensitively and requires a token boundary. */
static int sql_match_keyword(const char **cursor, const char *keyword) {
    const char *start = *cursor;
    const char *word = keyword;

    while (*word != '\0') {
        if (tolower((unsigned char)*start) != tolower((unsigned char)*word)) {
            return 0;
        }
        start++;
        word++;
    }

    if (isalnum((unsigned char)*start) || *start == '_') {
        return 0;
    }

    *cursor = start;
    return 1;
}

/* Returns the lock mode implied by the first SQL keyword. */
SQLLockMode sql_determine_lock_mode(const char *input) {
    const char *cursor;

    if (input == NULL) {
        return SQL_LOCK_NONE;
    }

    cursor = input;
    sql_skip_spaces(&cursor);

    if (sql_match_keyword(&cursor, "SELECT")) {
        return SQL_LOCK_READ;
    }

    if (sql_match_keyword(&cursor, "INSERT")) {
        return SQL_LOCK_WRITE;
    }

    return SQL_LOCK_NONE;
}

/* Matches a single expected punctuation character. */
static int sql_match_char(const char **cursor, char expected) {
    sql_skip_spaces(cursor);

    if (**cursor != expected) {
        return 0;
    }

    (*cursor)++;
    return 1;
}

/* Parses an identifier such as users, id, name, or age. */
static int sql_parse_identifier(const char **cursor, char *buffer, size_t buffer_size) {
    size_t length = 0;

    sql_skip_spaces(cursor);

    if (!(isalpha((unsigned char)**cursor) || **cursor == '_')) {
        return 0;
    }

    while ((isalnum((unsigned char)**cursor) || **cursor == '_') && length + 1 < buffer_size) {
        buffer[length++] = **cursor;
        (*cursor)++;
    }

    if (length == 0) {
        return 0;
    }

    while (isalnum((unsigned char)**cursor) || **cursor == '_') {
        (*cursor)++;
        length++;
    }

    if (length >= buffer_size) {
        return 0;
    }

    buffer[length] = '\0';
    return 1;
}

/* Parses a single-quoted string literal. */
static int sql_parse_string(const char **cursor, char *buffer, size_t buffer_size) {
    size_t length = 0;

    sql_skip_spaces(cursor);

    if (**cursor != '\'') {
        return 0;
    }

    (*cursor)++;

    while (**cursor != '\0' && **cursor != '\'') {
        if (length + 1 >= buffer_size) {
            return 0;
        }
        buffer[length++] = **cursor;
        (*cursor)++;
    }

    if (**cursor != '\'') {
        return 0;
    }

    (*cursor)++;
    buffer[length] = '\0';
    return 1;
}

/* Parses a signed integer literal. */
static int sql_parse_int(const char **cursor, int *value) {
    char *end_ptr;
    long parsed;

    sql_skip_spaces(cursor);

    if (!(isdigit((unsigned char)**cursor) || **cursor == '-')) {
        return 0;
    }

    parsed = strtol(*cursor, &end_ptr, 10);

    if (end_ptr == *cursor) {
        return 0;
    }

    *value = (int)parsed;
    *cursor = end_ptr;
    return 1;
}

/* Parses one of =, <, <=, >, >= into a comparison enum. */
static int sql_parse_comparison(const char **cursor, TableComparison *comparison) {
    sql_skip_spaces(cursor);

    if ((*cursor)[0] == '>' && (*cursor)[1] == '=') {
        *comparison = TABLE_COMPARISON_GE;
        *cursor += 2;
        return 1;
    }

    if ((*cursor)[0] == '<' && (*cursor)[1] == '=') {
        *comparison = TABLE_COMPARISON_LE;
        *cursor += 2;
        return 1;
    }

    if (**cursor == '>') {
        *comparison = TABLE_COMPARISON_GT;
        (*cursor)++;
        return 1;
    }

    if (**cursor == '<') {
        *comparison = TABLE_COMPARISON_LT;
        (*cursor)++;
        return 1;
    }

    if (**cursor == '=') {
        *comparison = TABLE_COMPARISON_EQ;
        (*cursor)++;
        return 1;
    }

    return 0;
}

/* Accepts an optional semicolon and trailing whitespace at the end. */
static int sql_match_statement_end(const char **cursor) {
    sql_skip_spaces(cursor);

    if (**cursor == ';') {
        (*cursor)++;
    }

    sql_skip_spaces(cursor);
    return **cursor == '\0';
}

static void sql_parse_result_init(SQLParseResult *result) {
    if (result == NULL) {
        return;
    }

    result->parsed = 0;
    result->command.type = SQL_COMMAND_NONE;
    result->command.comparison = TABLE_COMPARISON_EQ;
    result->command.table_name[0] = '\0';
    result->command.name[0] = '\0';
    result->command.selected_column[0] = '\0';
    result->command.where_column[0] = '\0';
    result->command.text_value[0] = '\0';
    result->command.int_value = 0;
    result->command.has_where = 0;
    result->status = SQL_STATUS_SYNTAX_ERROR;
    result->error_code = 0;
    result->sql_state[0] = '\0';
    result->error_message[0] = '\0';
}

static void sql_parse_result_set_error(SQLParseResult *result, SQLStatus status, int error_code, const char *sql_state, const char *message) {
    if (result == NULL) {
        return;
    }

    result->parsed = 0;
    result->command.type = SQL_COMMAND_NONE;
    result->status = status;
    result->error_code = error_code;
    snprintf(result->sql_state, sizeof(result->sql_state), "%s", sql_state);
    snprintf(result->error_message, sizeof(result->error_message), "%s", message);
}

static void sql_parse_set_syntax_error(SQLParseResult *result, const char *cursor) {
    char near_excerpt[64];

    sql_build_near_excerpt(cursor, near_excerpt, sizeof(near_excerpt));
    sql_parse_result_set_error(
        result,
        SQL_STATUS_SYNTAX_ERROR,
        1064,
        "42000",
        ""
    );

    if (result != NULL) {
        snprintf(
            result->error_message,
            sizeof(result->error_message),
            "ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your sql processor2 version for the right syntax to use near '%s' at line 1",
            near_excerpt
        );
    }
}

static void sql_parse_set_unknown_column_error(SQLParseResult *result, const char *column, const char *clause_name) {
    sql_parse_result_set_error(
        result,
        SQL_STATUS_QUERY_ERROR,
        1054,
        "42S22",
        ""
    );

    if (result != NULL) {
        snprintf(
            result->error_message,
            sizeof(result->error_message),
            "ERROR 1054 (42S22): Unknown column '%s' in '%s'",
            column,
            clause_name
        );
    }
}

static void sql_parse_insert(const char *input, SQLParseResult *result) {
    const char *cursor = input;
    char table_name[32];

    if (!sql_match_keyword(&cursor, "INSERT")) {
        return;
    }

    sql_skip_spaces(&cursor);
    if (!sql_match_keyword(&cursor, "INTO")) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_parse_identifier(&cursor, table_name, sizeof(table_name))) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (strcasecmp(table_name, "users") != 0) {
        sql_parse_set_syntax_error(result, table_name);
        return;
    }

    sql_skip_spaces(&cursor);
    if (!sql_match_keyword(&cursor, "VALUES")) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_match_char(&cursor, '(')) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_parse_string(&cursor, result->command.name, sizeof(result->command.name))) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_match_char(&cursor, ',')) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_parse_int(&cursor, &result->command.int_value)) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_match_char(&cursor, ')')) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_match_statement_end(&cursor)) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    snprintf(result->command.table_name, sizeof(result->command.table_name), "%s", table_name);
    result->command.type = SQL_COMMAND_INSERT;
    result->parsed = 1;
    result->status = SQL_STATUS_OK;
}

static void sql_parse_select(const char *input, SQLParseResult *result) {
    const char *cursor = input;
    char table_name[32];
    char column[32];
    char selected_column[32];
    const char *comparison_cursor;

    if (!sql_match_keyword(&cursor, "SELECT")) {
        return;
    }

    sql_skip_spaces(&cursor);
    if (!sql_match_char(&cursor, '*')) {
        if (!sql_parse_identifier(&cursor, selected_column, sizeof(selected_column))) {
            sql_parse_set_syntax_error(result, cursor);
            return;
        }

        if (!sql_is_known_column(selected_column)) {
            sql_parse_set_unknown_column_error(result, selected_column, "field list");
            return;
        }

        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    sql_skip_spaces(&cursor);
    if (!sql_match_keyword(&cursor, "FROM")) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_parse_identifier(&cursor, table_name, sizeof(table_name))) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (strcasecmp(table_name, "users") != 0) {
        sql_parse_set_syntax_error(result, table_name);
        return;
    }

    sql_skip_spaces(&cursor);
    if (sql_match_statement_end(&cursor)) {
        snprintf(result->command.table_name, sizeof(result->command.table_name), "%s", table_name);
        result->command.type = SQL_COMMAND_SELECT_ALL;
        result->parsed = 1;
        result->status = SQL_STATUS_OK;
        return;
    }

    if (!sql_match_keyword(&cursor, "WHERE")) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    if (!sql_parse_identifier(&cursor, column, sizeof(column))) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    comparison_cursor = cursor;
    if (!sql_parse_comparison(&cursor, &result->command.comparison)) {
        sql_parse_set_syntax_error(result, cursor);
        return;
    }

    snprintf(result->command.table_name, sizeof(result->command.table_name), "%s", table_name);
    snprintf(result->command.where_column, sizeof(result->command.where_column), "%s", column);
    result->command.has_where = 1;

    if (strcasecmp(column, "id") == 0) {
        if (!sql_parse_int(&cursor, &result->command.int_value) || !sql_match_statement_end(&cursor)) {
            sql_parse_set_syntax_error(result, cursor);
            return;
        }

        result->command.type = SQL_COMMAND_SELECT_BY_ID;
    } else if (strcasecmp(column, "name") == 0) {
        if (result->command.comparison != TABLE_COMPARISON_EQ) {
            sql_parse_set_syntax_error(result, comparison_cursor);
            return;
        }

        if (!sql_parse_string(&cursor, result->command.text_value, sizeof(result->command.text_value)) || !sql_match_statement_end(&cursor)) {
            sql_parse_set_syntax_error(result, cursor);
            return;
        }

        result->command.type = SQL_COMMAND_SELECT_BY_NAME;
    } else if (strcasecmp(column, "age") == 0) {
        if (!sql_parse_int(&cursor, &result->command.int_value) || !sql_match_statement_end(&cursor)) {
            sql_parse_set_syntax_error(result, cursor);
            return;
        }

        result->command.type = SQL_COMMAND_SELECT_BY_AGE;
    } else {
        sql_parse_set_unknown_column_error(result, column, "where clause");
        return;
    }

    result->parsed = 1;
    result->status = SQL_STATUS_OK;
}

/* Parses one SQL statement into a command object. */
int sql_parse(const char *input, SQLParseResult *result) {
    const char *cursor;

    if (result == NULL) {
        return 0;
    }

    sql_parse_result_init(result);

    if (input == NULL) {
        sql_parse_result_set_error(result, SQL_STATUS_ERROR, 0, "", "Internal database error");
        return 0;
    }

    cursor = input;
    sql_skip_spaces(&cursor);

    if (sql_match_keyword(&cursor, "EXIT") || sql_match_keyword(&cursor, "QUIT")) {
        if (sql_match_statement_end(&cursor)) {
            result->command.type = SQL_COMMAND_EXIT;
            result->parsed = 1;
            result->status = SQL_STATUS_EXIT;
            return 1;
        }

        sql_parse_set_syntax_error(result, cursor);
        return 0;
    }

    sql_parse_insert(input, result);
    if (result->parsed || result->error_message[0] != '\0') {
        return result->parsed;
    }

    sql_parse_select(input, result);
    if (result->parsed || result->error_message[0] != '\0') {
        return result->parsed;
    }

    sql_parse_set_syntax_error(result, cursor);
    return 0;
}

/* Returns the lock mode required for a parsed SQL command. */
SQLLockMode sql_command_lock_mode(const SQLCommand *command) {
    if (command == NULL) {
        return SQL_LOCK_NONE;
    }

    switch (command->type) {
        case SQL_COMMAND_INSERT:
            return SQL_LOCK_WRITE;
        case SQL_COMMAND_SELECT_ALL:
        case SQL_COMMAND_SELECT_BY_ID:
        case SQL_COMMAND_SELECT_BY_NAME:
        case SQL_COMMAND_SELECT_BY_AGE:
            return SQL_LOCK_READ;
        case SQL_COMMAND_EXIT:
        case SQL_COMMAND_NONE:
        default:
            return SQL_LOCK_NONE;
    }
}

static SQLResult sql_execute_insert_plan(Table *table, const SQLCommand *command) {
    SQLResult result = sql_make_result(SQL_STATUS_ERROR, SQL_ACTION_NONE);
    Record *record;

    if (table == NULL || command == NULL || command->type != SQL_COMMAND_INSERT) {
        return result;
    }

    record = table_insert(table, command->name, command->int_value);
    if (record == NULL) {
        return result;
    }

    result.status = SQL_STATUS_OK;
    result.action = SQL_ACTION_INSERT;
    result.record = record;
    result.inserted_id = record->id;
    result.row_count = 1;
    return result;
}

static SQLResult sql_execute_select_plan(Table *table, const SQLCommand *command) {
    SQLResult result = sql_make_result(SQL_STATUS_ERROR, SQL_ACTION_NONE);

    if (table == NULL || command == NULL) {
        return result;
    }

    switch (command->type) {
        case SQL_COMMAND_SELECT_ALL:
            if (!table_collect_all(table, &result.records, &result.row_count)) {
                return result;
            }
            break;
        case SQL_COMMAND_SELECT_BY_ID:
            if (!table_find_by_id_condition(table, command->comparison, command->int_value, &result.records, &result.row_count)) {
                return result;
            }
            break;
        case SQL_COMMAND_SELECT_BY_NAME:
            if (!table_find_by_name_matches(table, command->text_value, &result.records, &result.row_count)) {
                return result;
            }
            break;
        case SQL_COMMAND_SELECT_BY_AGE:
            if (!table_find_by_age_condition(table, command->comparison, command->int_value, &result.records, &result.row_count)) {
                return result;
            }
            break;
        default:
            return result;
    }

    result.record = (result.row_count > 0) ? result.records[0] : NULL;
    result.status = (result.row_count == 0) ? SQL_STATUS_NOT_FOUND : SQL_STATUS_OK;
    result.action = SQL_ACTION_SELECT_ROWS;
    return result;
}

/* Executes a parsed SQL command against the table. */
SQLResult sql_execute_plan(Table *table, const SQLCommand *command) {
    if (command == NULL) {
        return sql_make_result(SQL_STATUS_ERROR, SQL_ACTION_NONE);
    }

    switch (command->type) {
        case SQL_COMMAND_INSERT:
            return sql_execute_insert_plan(table, command);
        case SQL_COMMAND_SELECT_ALL:
        case SQL_COMMAND_SELECT_BY_ID:
        case SQL_COMMAND_SELECT_BY_NAME:
        case SQL_COMMAND_SELECT_BY_AGE:
            return sql_execute_select_plan(table, command);
        case SQL_COMMAND_EXIT: {
            SQLResult result = sql_make_result(SQL_STATUS_EXIT, SQL_ACTION_NONE);
            return result;
        }
        case SQL_COMMAND_NONE:
        default:
            return sql_make_result(SQL_STATUS_ERROR, SQL_ACTION_NONE);
    }
}

/* Parses one SQL statement and executes it against the table. */
SQLResult sql_execute(Table *table, const char *input) {
    SQLParseResult parse_result;
    SQLResult result;

    if (!sql_parse(input, &parse_result)) {
        result = sql_make_result(parse_result.status, SQL_ACTION_NONE);
        result.error_code = parse_result.error_code;
        snprintf(result.sql_state, sizeof(result.sql_state), "%s", parse_result.sql_state);
        snprintf(result.error_message, sizeof(result.error_message), "%s", parse_result.error_message);
        return result;
    }

    result = sql_execute_plan(table, &parse_result.command);
    if (parse_result.command.type == SQL_COMMAND_EXIT) {
        result.status = SQL_STATUS_EXIT;
    }
    return result;
}

/* Releases any heap memory owned by a SQL result. */
void sql_result_destroy(SQLResult *result) {
    if (result == NULL) {
        return;
    }

    free(result->records);
    result->records = NULL;
    result->record = NULL;
    result->inserted_id = 0;
    result->row_count = 0;
    result->action = SQL_ACTION_NONE;
    result->status = SQL_STATUS_OK;
    result->error_code = 0;
    result->sql_state[0] = '\0';
    result->error_message[0] = '\0';
}
