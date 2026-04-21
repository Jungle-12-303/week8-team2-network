#include "thread_pool.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "job_queue.h"

struct ThreadPool {
    pthread_t *workers;
    size_t worker_count;
    JobQueue queue;
    pthread_mutex_t mutex;
    pthread_cond_t has_jobs;
    pthread_cond_t has_space;
    int stopping;
    ThreadPoolHandler handler;
    void *context;
};

static void *thread_pool_worker_main(void *arg)
{
    ThreadPool *pool;
    int client_fd;

    pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->mutex);

        while (!pool->stopping && pool->queue.count == 0) {
            pthread_cond_wait(&pool->has_jobs, &pool->mutex);
        }

        if (pool->stopping && pool->queue.count == 0) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        job_queue_pop(&pool->queue, &client_fd);
        pthread_cond_signal(&pool->has_space);
        pthread_mutex_unlock(&pool->mutex);

        pool->handler(pool->context, client_fd);
    }

    return NULL;
}

ThreadPool *thread_pool_create(size_t worker_count, size_t queue_capacity,
    ThreadPoolHandler handler, void *context)
{
    ThreadPool *pool;
    size_t index;

    if (worker_count == 0 || queue_capacity == 0 || handler == NULL) {
        return NULL;
    }

    pool = (ThreadPool *)calloc(1, sizeof(ThreadPool));
    if (pool == NULL) {
        return NULL;
    }

    pool->workers = (pthread_t *)calloc(worker_count, sizeof(pthread_t));
    if (pool->workers == NULL) {
        free(pool);
        return NULL;
    }

    if (!job_queue_init(&pool->queue, queue_capacity)) {
        free(pool->workers);
        free(pool);
        return NULL;
    }

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->has_jobs, NULL);
    pthread_cond_init(&pool->has_space, NULL);
    pool->worker_count = worker_count;
    pool->handler = handler;
    pool->context = context;

    for (index = 0; index < worker_count; index++) {
        if (pthread_create(&pool->workers[index], NULL,
                thread_pool_worker_main, pool) != 0) {
            pool->stopping = 1;
            pool->worker_count = index;
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

void thread_pool_destroy(ThreadPool *pool)
{
    size_t index;
    int client_fd;

    if (pool == NULL) {
        return;
    }

    pthread_mutex_lock(&pool->mutex);
    pool->stopping = 1;
    pthread_cond_broadcast(&pool->has_jobs);
    pthread_cond_broadcast(&pool->has_space);
    pthread_mutex_unlock(&pool->mutex);

    for (index = 0; index < pool->worker_count; index++) {
        pthread_join(pool->workers[index], NULL);
    }

    while (job_queue_pop(&pool->queue, &client_fd)) {
        close(client_fd);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->has_jobs);
    pthread_cond_destroy(&pool->has_space);
    job_queue_destroy(&pool->queue);
    free(pool->workers);
    free(pool);
}

int thread_pool_enqueue(ThreadPool *pool, int client_fd)
{
    if (pool == NULL) {
        return 0;
    }

    pthread_mutex_lock(&pool->mutex);

    while (!pool->stopping && pool->queue.count == pool->queue.capacity) {
        pthread_cond_wait(&pool->has_space, &pool->mutex);
    }

    if (pool->stopping) {
        pthread_mutex_unlock(&pool->mutex);
        return 0;
    }

    if (!job_queue_push(&pool->queue, client_fd)) {
        pthread_mutex_unlock(&pool->mutex);
        return 0;
    }

    pthread_cond_signal(&pool->has_jobs);
    pthread_mutex_unlock(&pool->mutex);
    return 1;
}
