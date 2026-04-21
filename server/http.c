#include "http.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_all(int client_fd, const char *data, size_t length) {
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(client_fd, data + sent, length - sent, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        if (n == 0) {
            return 0;
        }
        sent += (size_t)n;
    }

    return 1;
}

static int read_until_header_end(int client_fd, char *buffer, size_t buffer_size, size_t *used, size_t *header_end) {
    while (*header_end == 0) {
        if (*used == buffer_size) {
            return 0;
        }

        {
            ssize_t n = recv(client_fd, buffer + *used, buffer_size - *used, 0);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return 0;
            }
            if (n == 0) {
                return 0;
            }
            *used += (size_t)n;
        }

        for (size_t index = 0; index + 3 < *used; index++) {
            if (buffer[index] == '\r' &&
                buffer[index + 1] == '\n' &&
                buffer[index + 2] == '\r' &&
                buffer[index + 3] == '\n') {
                *header_end = index;
                return 1;
            }
        }
    }

    return 1;
}

static void set_error(char *error_message, size_t error_message_size, const char *message) {
    if (error_message == NULL || error_message_size == 0) {
        return;
    }

    snprintf(error_message, error_message_size, "%s", message);
}

const char *http_status_reason(int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 413:
            return "Payload Too Large";
        case 500:
            return "Internal Server Error";
        case 503:
            return "Service Unavailable";
        default:
            return "OK";
    }
}

int http_read_request(int client_fd, HttpRequest *request, int *error_status, char *error_message, size_t error_message_size) {
    char buffer[HTTP_MAX_REQUEST_SIZE];
    size_t used = 0;
    size_t header_end = 0;
    char *headers;
    char *saveptr = NULL;
    size_t body_offset;
    size_t content_length = 0;
    size_t body_available;
    int saw_content_length = 0;

    if (request == NULL) {
        if (error_status != NULL) {
            *error_status = 500;
        }
        set_error(error_message, error_message_size, "Internal request error");
        return -1;
    }

    memset(request, 0, sizeof(*request));

    if (!read_until_header_end(client_fd, buffer, sizeof(buffer), &used, &header_end)) {
        if (error_status != NULL) {
            *error_status = 400;
        }
        set_error(error_message, error_message_size, "Malformed HTTP request");
        return -1;
    }

    buffer[header_end] = '\0';
    headers = strstr(buffer, "\r\n");
    if (headers == NULL) {
        if (error_status != NULL) {
            *error_status = 400;
        }
        set_error(error_message, error_message_size, "Malformed HTTP request line");
        return -1;
    }

    *headers = '\0';
    headers += 2;

    if (sscanf(buffer, "%15s %63s %15s", request->method, request->path, request->version) != 3) {
        if (error_status != NULL) {
            *error_status = 400;
        }
        set_error(error_message, error_message_size, "Malformed HTTP request line");
        return -1;
    }

    if (strcmp(request->method, "POST") != 0) {
        if (error_status != NULL) {
            *error_status = 405;
        }
        set_error(error_message, error_message_size, "Only POST is supported");
        return -1;
    }

    if (strcmp(request->path, "/query") != 0) {
        if (error_status != NULL) {
            *error_status = 404;
        }
        set_error(error_message, error_message_size, "Only /query is supported");
        return -1;
    }

    for (char *line = strtok_r(headers, "\r\n", &saveptr); line != NULL; line = strtok_r(NULL, "\r\n", &saveptr)) {
        char *colon;

        if (*line == '\0') {
            continue;
        }

        colon = strchr(line, ':');
        if (colon == NULL) {
            if (error_status != NULL) {
                *error_status = 400;
            }
            set_error(error_message, error_message_size, "Malformed HTTP header");
            return -1;
        }

        *colon = '\0';
        colon++;
        while (*colon != '\0' && isspace((unsigned char)*colon)) {
            colon++;
        }

        if (strcasecmp(line, "Content-Length") == 0) {
            char *end_ptr = NULL;
            unsigned long parsed = strtoul(colon, &end_ptr, 10);
            if (end_ptr == colon || *end_ptr != '\0') {
                if (error_status != NULL) {
                    *error_status = 400;
                }
                set_error(error_message, error_message_size, "Invalid Content-Length");
                return -1;
            }
            content_length = (size_t)parsed;
            saw_content_length = 1;
        }
    }

    if (!saw_content_length) {
        if (error_status != NULL) {
            *error_status = 400;
        }
        set_error(error_message, error_message_size, "Missing Content-Length");
        return -1;
    }

    if (content_length > HTTP_MAX_BODY_SIZE) {
        if (error_status != NULL) {
            *error_status = 413;
        }
        set_error(error_message, error_message_size, "Request body is too large");
        return -1;
    }

    request->content_length = content_length;
    body_offset = header_end + 4;
    body_available = (used > body_offset) ? (used - body_offset) : 0;

    if (body_available > content_length) {
        body_available = content_length;
    }

    if (body_available > 0) {
        memcpy(request->body, buffer + body_offset, body_available);
    }

    while (body_available < content_length) {
        ssize_t n = recv(client_fd, request->body + body_available, content_length - body_available, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (error_status != NULL) {
                *error_status = 400;
            }
            set_error(error_message, error_message_size, "Failed to read request body");
            return -1;
        }
        if (n == 0) {
            if (error_status != NULL) {
                *error_status = 400;
            }
            set_error(error_message, error_message_size, "Incomplete request body");
            return -1;
        }
        body_available += (size_t)n;
    }

    request->body[content_length] = '\0';
    return 0;
}

int http_send_response(int client_fd, int status_code, const char *content_type, const char *body) {
    char header[512];
    size_t body_length;
    int header_length;

    if (body == NULL) {
        body = "";
    }
    if (content_type == NULL) {
        content_type = "text/plain; charset=utf-8";
    }

    body_length = strlen(body);
    header_length = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code,
        http_status_reason(status_code),
        content_type,
        body_length
    );

    if (header_length < 0 || (size_t)header_length >= sizeof(header)) {
        return 0;
    }

    if (!send_all(client_fd, header, (size_t)header_length)) {
        return 0;
    }

    return send_all(client_fd, body, body_length);
}
