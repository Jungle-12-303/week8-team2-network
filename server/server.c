#include "server.h"

#include "api.h"
#include "http.h"
#include "json_util.h"
#include "thread_pool.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct Server {
    int listen_fd;
    Table *table;
    pthread_mutex_t db_mutex;
    ThreadPool pool;
    ServerConfig config;
    int initialized;
};

static volatile sig_atomic_t g_shutdown_requested = 0;

void server_signal_shutdown(void) {
    g_shutdown_requested = 1;
}

static int server_shutdown_requested(void) {
    return g_shutdown_requested != 0;
}

static int server_make_listen_socket(unsigned short port, int backlog) {
    int listen_fd;
    int reuse_addr = 1;
    struct sockaddr_in address;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        return -1;
    }

    (void)setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, backlog) < 0) {
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

static char *server_build_error_body(const char *status, const char *message) {
    JsonBuffer buffer;

    if (!json_buffer_init(&buffer, 128)) {
        return NULL;
    }

    if (!json_buffer_append(&buffer, "{\"ok\":false,\"status\":")) {
        json_buffer_destroy(&buffer);
        return NULL;
    }
    if (!json_buffer_append_json_string(&buffer, status)) {
        json_buffer_destroy(&buffer);
        return NULL;
    }
    if (!json_buffer_append(&buffer, ",\"message\":")) {
        json_buffer_destroy(&buffer);
        return NULL;
    }
    if (!json_buffer_append_json_string(&buffer, message)) {
        json_buffer_destroy(&buffer);
        return NULL;
    }
    if (!json_buffer_append(&buffer, "}")) {
        json_buffer_destroy(&buffer);
        return NULL;
    }

    return json_buffer_detach(&buffer);
}

static const char *server_http_error_status_name(int status_code) {
    switch (status_code) {
        case 404:
            return "not_found";
        case 405:
            return "method_not_allowed";
        case 413:
            return "payload_too_large";
        case 500:
            return "internal_error";
        case 503:
            return "queue_full";
        case 400:
        default:
            return "bad_request";
    }
}

static void server_handle_client(void *context, int client_fd) {
    Server *server = (Server *)context;
    HttpRequest request;
    ApiResult api_result;
    int error_status = 400;
    char error_message[128];

    error_message[0] = '\0';

    if (http_read_request(client_fd, &request, &error_status, error_message, sizeof(error_message)) != 0) {
        char *body = server_build_error_body(
            server_http_error_status_name(error_status),
            error_message[0] != '\0' ? error_message : "Bad request"
        );
        if (body != NULL) {
            http_send_response(client_fd, error_status, "application/json; charset=utf-8", body);
            free(body);
        }
        return;
    }

    if (!api_handle_query(server->table, &server->db_mutex, request.body, &api_result)) {
        char *body = server_build_error_body("internal_error", "Failed to execute SQL");
        if (body != NULL) {
            http_send_response(client_fd, 500, "application/json; charset=utf-8", body);
            free(body);
        }
        return;
    }

    http_send_response(client_fd, api_result.http_status, "application/json; charset=utf-8", api_result.body);
    api_result_destroy(&api_result);
}

Server *server_create(const ServerConfig *config) {
    Server *server;

    if (config == NULL) {
        return NULL;
    }

    server = (Server *)calloc(1, sizeof(Server));
    if (server == NULL) {
        return NULL;
    }

    memset(server, 0, sizeof(*server));
    server->listen_fd = -1;
    server->config = *config;

    server->table = table_create();
    if (server->table == NULL) {
        free(server);
        return NULL;
    }

    if (pthread_mutex_init(&server->db_mutex, NULL) != 0) {
        table_destroy(server->table);
        server->table = NULL;
        free(server);
        return NULL;
    }

    if (!thread_pool_init(&server->pool, config->worker_count, config->queue_capacity, server_handle_client, server)) {
        pthread_mutex_destroy(&server->db_mutex);
        table_destroy(server->table);
        server->table = NULL;
        free(server);
        return NULL;
    }

    server->listen_fd = server_make_listen_socket(config->port, config->backlog);
    if (server->listen_fd < 0) {
        thread_pool_destroy(&server->pool);
        pthread_mutex_destroy(&server->db_mutex);
        table_destroy(server->table);
        server->table = NULL;
        free(server);
        return NULL;
    }

    server->initialized = 1;
    return server;
}

int server_run(Server *server) {
    struct sockaddr_in client_address;
    socklen_t client_length = sizeof(client_address);

    if (server == NULL || !server->initialized) {
        return 0;
    }

    while (!server_shutdown_requested()) {
        int client_fd;

        client_length = sizeof(client_address);
        client_fd = accept(server->listen_fd, (struct sockaddr *)&client_address, &client_length);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (server_shutdown_requested()) {
                break;
            }
            break;
        }

        if (!thread_pool_submit(&server->pool, client_fd)) {
            char *body = server_build_error_body("queue_full", "Server is busy");
            if (body != NULL) {
                http_send_response(client_fd, 503, "application/json; charset=utf-8", body);
                free(body);
            }
            close(client_fd);
        }
    }

    thread_pool_shutdown(&server->pool);
    return 1;
}

void server_destroy(Server *server) {
    if (server == NULL) {
        return;
    }

    thread_pool_destroy(&server->pool);

    if (server->listen_fd >= 0) {
        close(server->listen_fd);
        server->listen_fd = -1;
    }

    pthread_mutex_destroy(&server->db_mutex);

    if (server->table != NULL) {
        table_destroy(server->table);
        server->table = NULL;
    }

    server->initialized = 0;
    free(server);
}
