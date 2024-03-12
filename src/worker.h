#ifndef TP2_WORKER_H
#define TP2_WORKER_H

#define BASE_PRIORITY_LEVEL 0
#define LOW_PRIORITY_LEVEL 1
#define HIGH_PRIORITY_LEVEL 2
#define NUM_PRIORITY_LEVELS 3

#include <pthread.h>

#include "ready_queue.h"

typedef struct worker worker_t;

struct worker {
    pthread_t thread;

    int core;
    ready_queue_t *ready_queue;
    int priority_level;
};

void *worker_run(void *user_data);

worker_t *worker_create(int core, ready_queue_t *ready_queue);

void worker_destroy(worker_t *worker);

void worker_join(worker_t *worker);

// Update the priority level of a worker given the status of a process
void update_priority_level(worker_t *worker, process_t *pProcess, int status);

// Computes the minimum of two integers
int min(int i, int i1);

// Computes the maximum of two integers
int max(int i, int i1);

// Update quantum with a dynamic leaky filter average given the recorded burst of a process
void *update_quantum(process_t *process, uint64_t *quantum);

#endif
