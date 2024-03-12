#ifndef TP2_WORKER_H
#define TP2_WORKER_H

#include <pthread.h>

#include "ready_queue.h"

typedef struct worker worker_t;

struct worker {
    pthread_t thread;

    int core;
    ready_queue_t *ready_queue;
};

void *worker_run(void *user_data);

worker_t *worker_create(int core, ready_queue_t *ready_queue);

void worker_destroy(worker_t *worker);

void worker_join(worker_t *worker);

// Update quantum with a dynamic leaky filter average given the recorded burst of a process
void update_quantums(process_t *process, uint64_t *quantum);

// Update the priority level of a process based on its status
void update_priority_level(process_t *process, int status);

// Return the minimum of two integers
int min(int a, int b);

// Return the maximum of two integers
int max(int a, int b);

#endif
