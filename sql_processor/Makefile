CC = cc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2
LDFLAGS = -pthread

COMMON_SRCS = bptree.c table.c sql.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

all: main perf_test condition_perf_test unit_test

main: main.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ main.o $(COMMON_OBJS) $(LDFLAGS)

perf_test: perf_test.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ perf_test.o $(COMMON_OBJS) $(LDFLAGS)

condition_perf_test: condition_perf_test.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ condition_perf_test.o $(COMMON_OBJS) $(LDFLAGS)

unit_test: unit_test.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ unit_test.o $(COMMON_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main perf_test condition_perf_test unit_test

.PHONY: all clean
