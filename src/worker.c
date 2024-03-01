#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

void *worker_run(void *user_data) {

    worker_t *worker = (worker_t *) user_data;
    int core = worker->core;
    ready_queue_t *ready_queue = worker->ready_queue;

    process_t *process = NULL;

    for (;;) {
        process = ready_queue_pop(ready_queue);

        if (process == NULL) break; // No more processes (poison pill)

        // TODO: ...
    }

    return NULL;
}

worker_t *worker_create(int core, ready_queue_t *ready_queue) {

    // TODO: Create a new worker and return it

    return NULL;
}

void worker_destroy(worker_t *worker) {

    // TODO: Destroy the worker
}

void worker_join(worker_t *worker) {

    // TODO: Join the worker
}
