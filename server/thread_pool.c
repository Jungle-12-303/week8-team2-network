#include "thread_pool.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int queue_debug_enabled(void) {
    static int checked = 0;
    static int enabled = 0;
    if (!checked) {
        const char *val = getenv("DEBUG_QUEUE");
        enabled = (val != NULL && strcmp(val, "1") == 0);
        checked = 1;
    }
    return enabled;
}

#define QUEUE_LOG(...) do { if (queue_debug_enabled()) { fprintf(stderr, "[QUEUE] " __VA_ARGS__); fflush(stderr); } } while (0)

static void *thread_pool_worker_main(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        int client_fd;

        pthread_mutex_lock(&pool->mutex);
        while (!pool->stop_requested && pool->size == 0) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if (pool->stop_requested && pool->size == 0) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        client_fd = pool->jobs[pool->head].client_fd;
        pool->head = (pool->head + 1) % pool->queue_capacity;
        pool->size--;
        QUEUE_LOG("dequeue  fd=%-4d size=%zu/%zu  worker=%lu\n",
                  client_fd, pool->size, pool->queue_capacity,
                  (unsigned long)pthread_self() % 10000);
        pthread_mutex_unlock(&pool->mutex);

        if (pool->handler != NULL) {
            pool->handler(pool->context, client_fd);
        }
        QUEUE_LOG("done     fd=%-4d (closed)\n", client_fd);
        close(client_fd);
    }

    return NULL;
}

int thread_pool_init(ThreadPool *pool, size_t worker_count, size_t queue_capacity, ThreadPoolJobHandler handler, void *context) {
    size_t index;

    if (pool == NULL || worker_count == 0 || queue_capacity == 0 || handler == NULL) {
        return 0;
    }

    pool->workers = (pthread_t *)calloc(worker_count, sizeof(pthread_t));
    pool->jobs = (ThreadPoolJob *)calloc(queue_capacity, sizeof(ThreadPoolJob));
    if (pool->workers == NULL || pool->jobs == NULL) {
        free(pool->workers);
        free(pool->jobs);
        return 0;
    }

    pool->worker_count = worker_count;
    pool->queue_capacity = queue_capacity;
    pool->head = 0;
    pool->tail = 0;
    pool->size = 0;
    pool->stop_requested = 0;
    pool->handler = handler;
    pool->context = context;

    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        free(pool->workers);
        free(pool->jobs);
        return 0;
    }

    if (pthread_cond_init(&pool->cond, NULL) != 0) {
        pthread_mutex_destroy(&pool->mutex);
        free(pool->workers);
        free(pool->jobs);
        return 0;
    }

    for (index = 0; index < worker_count; index++) {
        if (pthread_create(&pool->workers[index], NULL, thread_pool_worker_main, pool) != 0) {
            pool->stop_requested = 1;
            pthread_cond_broadcast(&pool->cond);
            while (index > 0) {
                index--;
                pthread_join(pool->workers[index], NULL);
            }
            pthread_cond_destroy(&pool->cond);
            pthread_mutex_destroy(&pool->mutex);
            free(pool->workers);
            free(pool->jobs);
            return 0;
        }
    }

    return 1;
}

int thread_pool_submit(ThreadPool *pool, int client_fd) {
    int accepted = 0;

    if (pool == NULL) {
        return 0;
    }

    pthread_mutex_lock(&pool->mutex);
    if (!pool->stop_requested && pool->size < pool->queue_capacity) {
        pool->jobs[pool->tail].client_fd = client_fd;
        pool->tail = (pool->tail + 1) % pool->queue_capacity;
        pool->size++;
        accepted = 1;
        QUEUE_LOG("submit   fd=%-4d size=%zu/%zu  head=%zu tail=%zu\n",
                  client_fd, pool->size, pool->queue_capacity,
                  pool->head, pool->tail);
        pthread_cond_signal(&pool->cond);
    }
    pthread_mutex_unlock(&pool->mutex);

    return accepted;
}

void thread_pool_shutdown(ThreadPool *pool) {
    if (pool == NULL || pool->workers == NULL) {
        return;
    }

    pthread_mutex_lock(&pool->mutex);
    pool->stop_requested = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
}

void thread_pool_destroy(ThreadPool *pool) {
    size_t index;

    if (pool == NULL || pool->workers == NULL) {
        return;
    }

    thread_pool_shutdown(pool);

    for (index = 0; index < pool->worker_count; index++) {
        pthread_join(pool->workers[index], NULL);
    }

    pthread_cond_destroy(&pool->cond);
    pthread_mutex_destroy(&pool->mutex);
    free(pool->workers);
    free(pool->jobs);
    pool->workers = NULL;
    pool->jobs = NULL;
    pool->worker_count = 0;
    pool->queue_capacity = 0;
    pool->head = 0;
    pool->tail = 0;
    pool->size = 0;
    pool->stop_requested = 0;
    pool->handler = NULL;
    pool->context = NULL;
}
