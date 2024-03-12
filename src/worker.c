#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

uint64_t quantum = 0; // In milliseconds

// Weight of the new recorded burst in the quantum average calculation
const float ALPHA_LOW = 0.4; // If the new recorded burst is lower than the current quantum
const float ALPHA_HIGH = 0.15; // If the new recorded burst is higher than the current quantum
const float GAMMA = 1.2; // Constant factor multiplying the new recorded burst


void *worker_run(void *user_data) {

    worker_t *worker = (worker_t *) user_data;
    ready_queue_t *ready_queue = worker->ready_queue;

    // Create the ready queues
    ready_queue_t *ready_queues[NUM_PRIORITY_LEVELS];
    // The first ready queue is the one passed as an argument. It's the base priority level.
    ready_queues[0] = ready_queue;
    for (int i = 1; i < NUM_PRIORITY_LEVELS; i++) {
        ready_queues[i] = malloc(sizeof(ready_queue_t));
        ready_queue_init(ready_queues[i]);
    }

    // Initialize the quantums
    uint64_t quantums[NUM_PRIORITY_LEVELS];
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        quantums[i] = quantum;
    }

    process_t *process = NULL;
    for (;;) {
        // Pop a process from the appropriate ready queue based on priority
        process = ready_queue_pop(ready_queues[worker->priority_level]);

        // No more processes (poison pill)
        if (process == NULL) break;

        // Get the quantum for the current priority level
        quantum = quantums[worker->priority_level];

        // Set the arrival time of the process
        uint64_t start_time = os_time();

        // Run the process
        int status = os_run_process(process, worker->core, quantum);

        // Update the burst length of the process
        process->burst_length += os_time() - start_time;

        // Adjust priority level based on process behavior
        update_priority_level(worker, process, status);

        // If the process was preempted or blocked, push it back to the ready queue
        if (status == OS_RUN_PREEMPTED) {
            ready_queue_push(ready_queues[worker->priority_level], process);
        } else if (status == OS_RUN_BLOCKED) {
            update_quantum(process, &quantum);
            os_start_io(process);
        }

    }

    for (int i = 1; i < NUM_PRIORITY_LEVELS; i++) {
        ready_queue_destroy(ready_queues[i]);
        free(ready_queues[i]);
    }

    return NULL;
}

void *update_quantum(process_t *process, uint64_t *quantum) {
    // If quantum is 0, initialize it with the recorded burst of the first process
    if (*quantum == 0) {
        *quantum = process->burst_length * GAMMA;
    } else {
        // Initialize alpha with ALPHA_LOW
        float alpha = ALPHA_LOW;
        // Give a higher weight to the new recorded burst if it is higher than the current quantum
        if (process->burst_length > *quantum) {
            alpha = ALPHA_HIGH;
        }
        // Update quantum with the new recorded burst
        *quantum = (1 - alpha) * *quantum + alpha * process->burst_length * GAMMA;
    }

    // Reset the burst length of the process
    process->burst_length = 0;
}

void update_priority_level(worker_t *worker, process_t *process, int status) {
    switch (status) {
        case OS_RUN_PREEMPTED:
            // Process was preempted, demote priority level
            worker->priority_level = max(worker->priority_level - 1, LOW_PRIORITY_LEVEL);
            break;
        case OS_RUN_BLOCKED:
            // Process was blocked, promote priority level
            worker->priority_level = min(worker->priority_level + 1, HIGH_PRIORITY_LEVEL);
            break;
        default:
            // Process completed its burst, reset priority level to base
            worker->priority_level = BASE_PRIORITY_LEVEL;
            break;

    }
}

int min(int i, int i1) {
    return i < i1 ? i : i1;
}

int max(int i, int i1) {
    return i > i1 ? i : i1;
}

worker_t *worker_create(int core, ready_queue_t *ready_queue) {
    // Allocate memory for the worker
    worker_t *worker = malloc(sizeof(worker_t));

    // Initialize the worker
    worker->core = core;
    worker->ready_queue = ready_queue;
    worker->priority_level = BASE_PRIORITY_LEVEL;

    // Create the worker thread and return the worker
    pthread_create(&worker->thread, NULL, worker_run, worker);
    return worker;
}

void worker_destroy(worker_t *worker) {
    pthread_join(worker->thread, NULL);
    free(worker);
}

void worker_join(worker_t *worker) {
    pthread_join(worker->thread, NULL);
}
