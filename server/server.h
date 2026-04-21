#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <stddef.h>

typedef struct ServerConfig {
    unsigned short port;
    size_t worker_count;
    size_t queue_capacity;
    int backlog;
} ServerConfig;

typedef struct Server Server;

Server *server_create(const ServerConfig *config);
int server_run(Server *server);
void server_destroy(Server *server);
void server_signal_shutdown(void);

#endif
