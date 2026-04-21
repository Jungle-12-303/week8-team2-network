CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g -O0 -pthread
INCLUDES = -I./src -I./third_party/team7_engine

SRC = \
    src/main.c \
    src/server/server.c \
    src/db/db_adapter.c \
    src/concurrency/job_queue.c \
    src/concurrency/thread_pool.c \
    third_party/team7_engine/bptree.c \
    third_party/team7_engine/table.c \
    third_party/team7_engine/sql.c

OBJ = $(SRC:.c=.o)
TARGET = mini_dbms_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

test: $(TARGET)
	bash tests/smoke_test.sh

.PHONY: all clean test
