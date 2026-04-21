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

    server = server_create(&config);
    if (server == NULL) {
        fprintf(stderr, "Failed to initialize server: %s\n", server_last_error());
        return 1;
    }

    printf("Listening on port %u\n", config.port);
    server_run(server);
    server_destroy(server);
    return 0;
}
