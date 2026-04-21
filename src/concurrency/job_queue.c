#include "job_queue.h"

#include <stdlib.h>

int job_queue_init(JobQueue *queue, size_t capacity)
{
    if (queue == NULL || capacity == 0) {
        return 0;
    }

    queue->items = (int *)calloc(capacity, sizeof(int));
    if (queue->items == NULL) {
        return 0;
    }

    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    return 1;
}

void job_queue_destroy(JobQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    free(queue->items);
    queue->items = NULL;
    queue->capacity = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

int job_queue_push(JobQueue *queue, int client_fd)
{
    if (queue == NULL || queue->count == queue->capacity) {
        return 0;
    }

    queue->items[queue->tail] = client_fd;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    return 1;
}

int job_queue_pop(JobQueue *queue, int *client_fd)
{
    if (queue == NULL || client_fd == NULL || queue->count == 0) {
        return 0;
    }

    *client_fd = queue->items[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    return 1;
}
