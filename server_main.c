#include "server/server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void handle_signal(int signal_number) {
    (void)signal_number;
    server_signal_shutdown();
}

static int install_signal_handlers(void) {
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) != 0) {
        return 0;
    }

    if (sigaction(SIGTERM, &action, NULL) != 0) {
        return 0;
    }

    return 1;
}

int main(int argc, char **argv) {
    Server *server;
    ServerConfig config;
    unsigned long port = 8080;

    if (argc > 1) {
        char *end_ptr = NULL;
        port = strtoul(argv[1], &end_ptr, 10);
        if (end_ptr == argv[1] || *end_ptr != '\0' || port == 0 || port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", argv[1]);
            return 1;
        }
    }

    if (!install_signal_handlers()) {
        fprintf(stderr, "Failed to install signal handlers.\n");
        return 1;
    }

    config.port = (unsigned short)port;
    config.worker_count = 4;
    config.queue_capacity = 16;
    config.backlog = 32;

    if (argc > 2) {
        char *end_ptr = NULL;
        unsigned long v = strtoul(argv[2], &end_ptr, 10);
        if (end_ptr == argv[2] || *end_ptr != '\0' || v == 0) {
            fprintf(stderr, "Invalid worker_count: %s\n", argv[2]);
            return 1;
        }
        config.worker_count = (size_t)v;
    }

    if (argc > 3) {
        char *end_ptr = NULL;
        unsigned long v = strtoul(argv[3], &end_ptr, 10);
        if (end_ptr == argv[3] || *end_ptr != '\0' || v == 0) {
            fprintf(stderr, "Invalid queue_capacity: %s\n", argv[3]);
            return 1;
        }
        config.queue_capacity = (size_t)v;
    }

    if (argc > 4) {
        char *end_ptr = NULL;
        unsigned long v = strtoul(argv[4], &end_ptr, 10);
        if (end_ptr == argv[4] || *end_ptr != '\0' || v == 0 || v > 128) {
            fprintf(stderr, "Invalid backlog: %s\n", argv[4]);
            return 1;
        }
        config.backlog = (int)v;
    }

    server = server_create(&config);
    if (server == NULL) {
        fprintf(stderr, "Failed to initialize server.\n");
        return 1;
    }

    printf("Listening on port %u (workers=%zu queue=%zu backlog=%d)\n",
           config.port, config.worker_count, config.queue_capacity, config.backlog);
    server_run(server);
    server_destroy(server);
    return 0;
}
