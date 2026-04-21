#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h>

typedef void (*ThreadPoolHandler)(void *context, int client_fd);

typedef struct ThreadPool ThreadPool;

ThreadPool *thread_pool_create(size_t worker_count, size_t queue_capacity,
    ThreadPoolHandler handler, void *context);
void thread_pool_destroy(ThreadPool *pool);
int thread_pool_enqueue(ThreadPool *pool, int client_fd);

#endif /* THREAD_POOL_H */
