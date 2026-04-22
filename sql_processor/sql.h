#ifndef SQL_H
#define SQL_H

#include "table.h"
#include <stddef.h>

#define SQL_SQLSTATE_SIZE 6
#define SQL_ERROR_MESSAGE_SIZE 256

typedef enum SQLStatus {
    SQL_STATUS_OK,
    SQL_STATUS_NOT_FOUND,
    SQL_STATUS_SYNTAX_ERROR,
    SQL_STATUS_QUERY_ERROR,
    SQL_STATUS_EXIT,
    SQL_STATUS_ERROR
} SQLStatus;

typedef enum SQLAction {
    SQL_ACTION_NONE,
    SQL_ACTION_INSERT,
    SQL_ACTION_SELECT_ROWS
} SQLAction;

typedef enum SQLLockMode {
    SQL_LOCK_NONE,
    SQL_LOCK_READ,
    SQL_LOCK_WRITE
} SQLLockMode;

typedef enum SQLCommandType {
    SQL_COMMAND_NONE,
    SQL_COMMAND_INSERT,
    SQL_COMMAND_SELECT_ALL,
    SQL_COMMAND_SELECT_BY_ID,
    SQL_COMMAND_SELECT_BY_NAME,
    SQL_COMMAND_SELECT_BY_AGE,
    SQL_COMMAND_EXIT
} SQLCommandType;

typedef struct SQLCommand {
    SQLCommandType type;
    TableComparison comparison;
    char table_name[32];
    char name[RECORD_NAME_SIZE];
    char selected_column[32];
    char where_column[32];
    char text_value[RECORD_NAME_SIZE];
    int int_value;
    int has_where;
} SQLCommand;

typedef struct SQLParseResult {
    int parsed;
    SQLCommand command;
    SQLStatus status;
    int error_code;
    char sql_state[SQL_SQLSTATE_SIZE];
    char error_message[SQL_ERROR_MESSAGE_SIZE];
} SQLParseResult;

typedef struct SQLResult {
    SQLStatus status;
    SQLAction action;
    Record *record;
    Record **records;
    int inserted_id;
    size_t row_count;
    int error_code;
    char sql_state[SQL_SQLSTATE_SIZE];
    char error_message[SQL_ERROR_MESSAGE_SIZE];
} SQLResult;

/* Parses one SQL statement into a command object. */
int sql_parse(const char *input, SQLParseResult *result);

/* Returns the lock mode required for a parsed SQL command. */
SQLLockMode sql_command_lock_mode(const SQLCommand *command);

/* Executes a parsed SQL command against the table. */
SQLResult sql_execute_plan(Table *table, const SQLCommand *command);

/* Parses one SQL statement and executes it against the table. */
SQLResult sql_execute(Table *table, const char *input);

/* Returns the lock mode required for the given SQL statement. */
SQLLockMode sql_determine_lock_mode(const char *input);

/* Releases any heap memory owned by a SQL result. */
void sql_result_destroy(SQLResult *result);

#endif
