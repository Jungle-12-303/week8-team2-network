#include "server/server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//즉 Ctrl+C나 종료 신호가 들어왔을 때 서버가 바로 죽는 대신, 종료 플래그를 세워서 안전하게 내려가게 하려는 목적입니다.
static void handle_signal(int signal_number) {
    (void)signal_number;
    server_signal_shutdown();
}

//종료신호 등록 : 프로그램이 종료 요청을 받았을 때 아무렇게나 끝나지 말고, 우리가 정한 종료 함수로 들어가게 연결해두는 것
static int install_signal_handlers(void) {
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // SIGINT : 터미널에서 Ctrl + C 눌렀을 때
    if (sigaction(SIGINT, &action, NULL) != 0) {
        return 0;
    }

    // SIGTERM : 프로세스 정상 종료 요청 시
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
    config.worker_count = 8;
    config.queue_capacity = 16;
    config.backlog = 32;

    server = server_create(&config);
    if (server == NULL) {
        fprintf(stderr, "Failed to initialize server.\n");
        return 1;
    }

    printf("Listening on port %u\n", config.port);
    server_run(server);
    server_destroy(server);
    return 0;
}
