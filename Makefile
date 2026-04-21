CC = cc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor
LDFLAGS = -pthread

SERVER_SRCS = \
	server_main.c \
	server/json_util.c \
	server/http.c \
	server/thread_pool.c \
	server/api.c \
	server/server.c \
	sql_processor/bptree.c \
	sql_processor/table.c \
	sql_processor/sql.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o)

all: db_server

db_server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) db_server

.PHONY: all clean
