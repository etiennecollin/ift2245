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

void* worker_run(void* user_data);

worker_t* worker_create(int core, ready_queue_t *ready_queue);

void worker_destroy(worker_t *worker);

void worker_join(worker_t *worker);

#endif
