#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../concurrency/thread_pool.h"

#define SERVER_MAX_REQUEST_SIZE 16384
#define SERVER_MAX_RESPONSE_SIZE 65536

typedef struct HttpRequest {
    char method[8];
    char path[256];
    const char *body;
    size_t body_length;
} HttpRequest;

typedef struct ServerContext {
    DbHandle *db;
} ServerContext;

static const char *find_case_insensitive(const char *haystack,
    const char *needle)
{
    size_t needle_length;

    needle_length = strlen(needle);
    if (needle_length == 0) {
        return haystack;
    }

    while (*haystack != '\0') {
        if (strncasecmp(haystack, needle, needle_length) == 0) {
            return haystack;
        }
        haystack++;
    }

    return NULL;
}

static int send_all(int client_fd, const char *buffer, size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t chunk;

        chunk = send(client_fd, buffer + written, length - written, 0);
        if (chunk < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }

        written += (size_t)chunk;
    }

    return 1;
}

static int send_json_response(int client_fd, int status_code,
    const char *status_text, const char *body)
{
    char response[SERVER_MAX_RESPONSE_SIZE];
    int body_length;
    int length;

    body_length = (int)strlen(body);
    length = snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code, status_text, body_length, body);
    if (length < 0 || (size_t)length >= sizeof(response)) {
        return 0;
    }

    return send_all(client_fd, response, (size_t)length);
}

static int find_header_end(const char *buffer, size_t length)
{
    size_t index;

    if (length < 4) {
        return -1;
    }

    for (index = 0; index + 3 < length; index++) {
        if (buffer[index] == '\r' && buffer[index + 1] == '\n' &&
            buffer[index + 2] == '\r' && buffer[index + 3] == '\n') {
            return (int)(index + 4);
        }
    }

    return -1;
}

static int parse_content_length(const char *headers, int *content_length)
{
    const char *cursor;
    char *end_ptr;
    long value;

    *content_length = 0;
    cursor = headers;

    while ((cursor = find_case_insensitive(cursor, "Content-Length:")) != NULL) {
        cursor += strlen("Content-Length:");
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        value = strtol(cursor, &end_ptr, 10);
        if (end_ptr == cursor || value < 0 ||
            value > SERVER_MAX_REQUEST_SIZE) {
            return 0;
        }

        *content_length = (int)value;
        return 1;
    }

    return 1;
}

static int read_http_request(int client_fd, char *buffer, size_t buffer_size,
    size_t *request_length)
{
    size_t used = 0;
    int header_end = -1;
    int content_length = 0;

    while (used + 1 < buffer_size) {
        ssize_t bytes_read;

        bytes_read = recv(client_fd, buffer + used,
            buffer_size - used - 1, 0);
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }

        used += (size_t)bytes_read;
        buffer[used] = '\0';

        if (header_end < 0) {
            header_end = find_header_end(buffer, used);
            if (header_end >= 0) {
                if (!parse_content_length(buffer, &content_length)) {
                    return 0;
                }
            }
        }

        if (header_end >= 0 &&
            used >= (size_t)(header_end + content_length)) {
            *request_length = used;
            return 1;
        }
    }

    return 0;
}

static int parse_request_line(char *line, HttpRequest *request)
{
    char version[16];

    if (sscanf(line, "%7s %255s %15s",
            request->method, request->path, version) != 3) {
        return 0;
    }

    return strcmp(version, "HTTP/1.1") == 0 ||
        strcmp(version, "HTTP/1.0") == 0;
}

static int parse_http_request(char *buffer, HttpRequest *request)
{
    char *line_end;
    int content_length = 0;

    memset(request, 0, sizeof(*request));

    line_end = strstr(buffer, "\r\n");
    if (line_end == NULL) {
        return 0;
    }

    *line_end = '\0';
    if (!parse_request_line(buffer, request)) {
        return 0;
    }

    if (!parse_content_length(line_end + 2, &content_length)) {
        return 0;
    }

    request->body = strstr(line_end + 2, "\r\n\r\n");
    if (request->body == NULL) {
        return 0;
    }

    request->body += 4;
    request->body_length = (size_t)content_length;
    return 1;
}

static int json_extract_string_field(const char *json, const char *field,
    char *out, size_t out_size)
{
    char pattern[64];
    const char *cursor;
    const char *start;
    const char *end;
    size_t length;

    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    cursor = strstr(json, pattern);
    if (cursor == NULL) {
        return 0;
    }

    cursor += strlen(pattern);
    while (*cursor == ' ' || *cursor == '\t' ||
        *cursor == '\n' || *cursor == '\r') {
        cursor++;
    }
    if (*cursor != ':') {
        return 0;
    }

    cursor++;
    while (*cursor == ' ' || *cursor == '\t' ||
        *cursor == '\n' || *cursor == '\r') {
        cursor++;
    }
    if (*cursor != '"') {
        return 0;
    }

    start = ++cursor;
    end = strchr(start, '"');
    if (end == NULL) {
        return 0;
    }

    length = (size_t)(end - start);
    if (length == 0 || length >= out_size) {
        return 0;
    }

    memcpy(out, start, length);
    out[length] = '\0';
    return 1;
}

static int json_extract_int_field(const char *json, const char *field,
    int *out)
{
    char pattern[64];
    const char *cursor;
    char *end_ptr;
    long value;

    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    cursor = strstr(json, pattern);
    if (cursor == NULL) {
        return 0;
    }

    cursor += strlen(pattern);
    while (*cursor == ' ' || *cursor == '\t' ||
        *cursor == '\n' || *cursor == '\r') {
        cursor++;
    }
    if (*cursor != ':') {
        return 0;
    }

    cursor++;
    while (*cursor == ' ' || *cursor == '\t' ||
        *cursor == '\n' || *cursor == '\r') {
        cursor++;
    }

    value = strtol(cursor, &end_ptr, 10);
    if (end_ptr == cursor) {
        return 0;
    }

    *out = (int)value;
    return 1;
}

static int append_rows_json(char *body, size_t body_size,
    const DbResult *result)
{
    size_t offset = 0;
    size_t index;
    int written;

    written = snprintf(body, body_size, "{\"ok\":true,\"rows\":[");
    if (written < 0 || (size_t)written >= body_size) {
        return 0;
    }
    offset = (size_t)written;

    for (index = 0; index < result->row_count; index++) {
        written = snprintf(body + offset, body_size - offset,
            "%s{\"id\":%d,\"name\":\"%s\",\"age\":%d}",
            (index == 0) ? "" : ",",
            result->rows[index].id,
            result->rows[index].name,
            result->rows[index].age);
        if (written < 0 || (size_t)written >= body_size - offset) {
            return 0;
        }
        offset += (size_t)written;
    }

    written = snprintf(body + offset, body_size - offset,
        "],\"row_count\":%zu}", result->row_count);
    if (written < 0 || (size_t)written >= body_size - offset) {
        return 0;
    }

    return 1;
}

static int handle_users_post(ServerContext *context, int client_fd,
    const HttpRequest *request)
{
    char name[64];
    int age;
    char sql[256];
    DbResult result;

    if (!json_extract_string_field(request->body, "name",
            name, sizeof(name)) ||
        !json_extract_int_field(request->body, "age", &age)) {
        return send_json_response(client_fd, 400, "Bad Request",
            "{\"ok\":false,\"error\":\"invalid JSON body\"}");
    }

    snprintf(sql, sizeof(sql),
        "INSERT INTO users VALUES ('%s', %d);", name, age);
    if (!db_execute_sql(context->db, sql, &result)) {
        return send_json_response(client_fd, 500,
            "Internal Server Error",
            "{\"ok\":false,\"error\":\"database execution failed\"}");
    }

    if (result.status != DB_STATUS_OK) {
        db_result_destroy(&result);
        return send_json_response(client_fd, 500,
            "Internal Server Error",
            "{\"ok\":false,\"error\":\"insert failed\"}");
    }

    db_result_destroy(&result);
    return send_json_response(client_fd, 201, "Created",
        "{\"ok\":true,\"message\":\"created\"}");
}

static int handle_users_get(ServerContext *context, int client_fd,
    const HttpRequest *request)
{
    char sql[256];
    char body[SERVER_MAX_RESPONSE_SIZE / 2];
    DbResult result;
    const char *id_path;
    char *end_ptr;
    long id;

    if (strcmp(request->path, "/users") == 0) {
        snprintf(sql, sizeof(sql), "SELECT * FROM users;");
    } else {
        id_path = request->path + strlen("/users/");
        id = strtol(id_path, &end_ptr, 10);
        if (*id_path == '\0' || *end_ptr != '\0' || id < 0) {
            return send_json_response(client_fd, 400, "Bad Request",
                "{\"ok\":false,\"error\":\"invalid user id\"}");
        }
        snprintf(sql, sizeof(sql),
            "SELECT * FROM users WHERE id = %ld;", id);
    }

    if (!db_execute_sql(context->db, sql, &result)) {
        return send_json_response(client_fd, 500,
            "Internal Server Error",
            "{\"ok\":false,\"error\":\"database execution failed\"}");
    }

    if (result.status == DB_STATUS_NOT_FOUND) {
        db_result_destroy(&result);
        return send_json_response(client_fd, 404, "Not Found",
            "{\"ok\":false,\"error\":\"user not found\"}");
    }

    if (result.status != DB_STATUS_OK) {
        db_result_destroy(&result);
        return send_json_response(client_fd, 500,
            "Internal Server Error",
            "{\"ok\":false,\"error\":\"query failed\"}");
    }

    if (!append_rows_json(body, sizeof(body), &result)) {
        db_result_destroy(&result);
        return send_json_response(client_fd, 500,
            "Internal Server Error",
            "{\"ok\":false,\"error\":\"response too large\"}");
    }

    db_result_destroy(&result);
    return send_json_response(client_fd, 200, "OK", body);
}

static void server_handle_client(void *arg, int client_fd)
{
    ServerContext *context;
    char request_buffer[SERVER_MAX_REQUEST_SIZE];
    size_t request_length = 0;
    HttpRequest request;

    context = (ServerContext *)arg;

    if (!read_http_request(client_fd, request_buffer,
            sizeof(request_buffer), &request_length)) {
        send_json_response(client_fd, 400, "Bad Request",
            "{\"ok\":false,\"error\":\"invalid HTTP request\"}");
        close(client_fd);
        return;
    }

    request_buffer[request_length] = '\0';
    if (!parse_http_request(request_buffer, &request)) {
        send_json_response(client_fd, 400, "Bad Request",
            "{\"ok\":false,\"error\":\"invalid HTTP request\"}");
        close(client_fd);
        return;
    }

    if (strcmp(request.method, "GET") == 0 &&
        strcmp(request.path, "/health") == 0) {
        send_json_response(client_fd, 200, "OK", "{\"ok\":true}");
        close(client_fd);
        return;
    }

    if (strcmp(request.method, "POST") == 0 &&
        strcmp(request.path, "/users") == 0) {
        handle_users_post(context, client_fd, &request);
        close(client_fd);
        return;
    }

    if (strcmp(request.method, "GET") == 0 &&
        strcmp(request.path, "/users") == 0) {
        handle_users_get(context, client_fd, &request);
        close(client_fd);
        return;
    }

    if (strcmp(request.method, "GET") == 0 &&
        strncmp(request.path, "/users/", 7) == 0) {
        handle_users_get(context, client_fd, &request);
        close(client_fd);
        return;
    }

    send_json_response(client_fd, 404, "Not Found",
        "{\"ok\":false,\"error\":\"route not found\"}");
    close(client_fd);
}

int server_run(const ServerConfig *config, DbHandle *db)
{
    int server_fd;
    int reuse_addr = 1;
    struct sockaddr_in address;
    ThreadPool *pool;
    ServerContext context;

    if (config == NULL || db == NULL) {
        return 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 0;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
            &reuse_addr, sizeof(reuse_addr)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 0;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((unsigned short)config->port);

    if (bind(server_fd, (struct sockaddr *)&address,
            sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return 0;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        close(server_fd);
        return 0;
    }

    context.db = db;
    pool = thread_pool_create((size_t)config->worker_count,
        (size_t)config->queue_capacity, server_handle_client, &context);
    if (pool == NULL) {
        close(server_fd);
        return 0;
    }

    printf("Server listening on port %d\n", config->port);

    while (1) {
        int client_fd;

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            break;
        }

        if (!thread_pool_enqueue(pool, client_fd)) {
            close(client_fd);
        }
    }

    thread_pool_destroy(pool);
    close(server_fd);
    return 1;
}
