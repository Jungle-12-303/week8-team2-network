#ifndef SERVER_THREAD_POOL_H
#define SERVER_THREAD_POOL_H

#include <stddef.h>
#include <pthread.h>

typedef void (*ThreadPoolJobHandler)(void *context, int client_fd);

typedef struct ThreadPoolJob {
    int client_fd;
} ThreadPoolJob;

typedef struct ThreadPool {
    pthread_t *workers;
    size_t worker_count;
    ThreadPoolJob *jobs;
    size_t queue_capacity;
    size_t head;
    size_t tail;
    size_t size;
    int stop_requested;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    ThreadPoolJobHandler handler;
    void *context;
} ThreadPool;

int thread_pool_init(ThreadPool *pool, size_t worker_count, size_t queue_capacity, ThreadPoolJobHandler handler, void *context);
int thread_pool_submit(ThreadPool *pool, int client_fd);
void thread_pool_shutdown(ThreadPool *pool);
void thread_pool_destroy(ThreadPool *pool);

#endif
