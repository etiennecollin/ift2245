#include "ready_queue.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "os.h"

void ready_queue_init(ready_queue_t *queue) {

    // TODO: ...
}

void ready_queue_destroy(ready_queue_t *queue) {

    // TODO: ...
}

void ready_queue_push(ready_queue_t *queue, process_t *process) {

    // TODO: ...
}

process_t *ready_queue_pop(ready_queue_t *queue) {

    // TODO: ...

    return NULL;
}

size_t ready_queue_size(ready_queue_t *queue) {

    // TODO: ...

    return 0;
}