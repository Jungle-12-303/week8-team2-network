#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <stddef.h>

typedef struct JobQueue {
    int *items;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} JobQueue;

int job_queue_init(JobQueue *queue, size_t capacity);
void job_queue_destroy(JobQueue *queue);
int job_queue_push(JobQueue *queue, int client_fd);
int job_queue_pop(JobQueue *queue, int *client_fd);

#endif /* JOB_QUEUE_H */
