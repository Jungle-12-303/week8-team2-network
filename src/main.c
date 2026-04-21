#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "db/db_adapter.h"
#include "server/server.h"

int main(int argc, char **argv)
{
    DbHandle *db;
    ServerConfig config;

    signal(SIGPIPE, SIG_IGN);

    config.port = 8080;
    config.worker_count = 4;
    config.queue_capacity = 64;

    if (argc > 1) {
        config.port = atoi(argv[1]);
    }

    db = db_init();
    if (db == NULL) {
        fprintf(stderr, "failed to initialize database\n");
        return 1;
    }

    if (!server_run(&config, db)) {
        db_shutdown(db);
        return 1;
    }

    db_shutdown(db);
    return 0;
}
