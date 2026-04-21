#ifndef SERVER_H
#define SERVER_H

#include "../db/db_adapter.h"

typedef struct ServerConfig {
    int port;
    int worker_count;
    int queue_capacity;
} ServerConfig;

int server_run(const ServerConfig *config, DbHandle *db);

#endif /* SERVER_H */
