#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <stddef.h>

#define HTTP_MAX_METHOD_SIZE 16
#define HTTP_MAX_PATH_SIZE 64
#define HTTP_MAX_VERSION_SIZE 16
#define HTTP_MAX_BODY_SIZE 4096
#define HTTP_MAX_REQUEST_SIZE 8192
#define HTTP_SOCKET_IO_TIMEOUT_SECONDS 5

typedef struct HttpRequest {
    char method[HTTP_MAX_METHOD_SIZE];
    char path[HTTP_MAX_PATH_SIZE];
    char version[HTTP_MAX_VERSION_SIZE];
    size_t content_length;
    char body[HTTP_MAX_BODY_SIZE + 1];
} HttpRequest;

int http_read_request(int client_fd, HttpRequest *request, int *error_status, char *error_message, size_t error_message_size);
int http_send_response(int client_fd, int status_code, const char *content_type, const char *body);
const char *http_status_reason(int status_code);

#endif
