#ifndef TP2_WORKER_H
#define TP2_WORKER_H

#include <pthread.h>

#include "ready_queue.h"

typedef struct worker worker_t;
typedef struct queue_process_data queue_process_data_t;

struct worker {
    pthread_t thread;

    int core;
    ready_queue_t *ready_queue;
};

struct queue_process_data {
    ready_queue_t *ready_queue;
    process_t *process;
};

void *worker_run(void *user_data);

worker_t *worker_create(int core, ready_queue_t *ready_queue);

void worker_destroy(worker_t *worker);

void worker_join(worker_t *worker);

// Update the priority level of a process based on its status
void update_priority_level(process_t *process, ready_queue_t *ready_queue);

// Return the quantum for a given priority level
uint64_t get_quantum(ready_queue_t *queue, int priority);

// Return the minimum of two integers
int min(int a, int b);

// Return the maximum of two integers
int max(int a, int b);

#endif
