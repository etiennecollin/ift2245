#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

const uint64_t INITIAL_QUANTUM = 0; // Initial quantum for all priority levels

// Weight of the new recorded burst in the quantum average calculation
const float ALPHA_LOW = 0.4; // If the new recorded burst is lower than the current quantum
const float ALPHA_HIGH = 0.15; // If the new recorded burst is higher than the current quantum

// Constant factor multiplying the new recorded burst
// The higher the priority level, the lower the constant factor
// The order of the constants is {BASE_PRIORITY, LOW_PRIORITY, HIGH_PRIORITY}
const float CONSTANTS[] = {1, 1.1, 0.9}; // Constant factor multiplying the new recorded burst


void *worker_run(void *user_data) {
    worker_t *worker = (worker_t *) user_data;
    ready_queue_t *ready_queue = worker->ready_queue;

    // Initialize the quantums
    uint64_t quantums[NUM_PRIORITY_LEVELS];
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        quantums[i] = INITIAL_QUANTUM;
    }

    process_t *process = NULL;
    uint64_t quantum;
    for (;;) {
        // Pop a process from the ready queue
        process = ready_queue_pop(ready_queue);

        // No more processes (poison pill)
        if (process == NULL) break;

        // Get the quantum for the current priority level
        quantum = quantums[process->priority_level];

        // Set the arrival time of the process
        uint64_t start_time = os_time();

        // Run the process
        int status = os_run_process(process, worker->core, quantum);

        // Update the burst length of the process
        process->burst_length += os_time() - start_time;

        // Adjust priority level based on process behavior
        update_priority_level(process, status);

        // If the process was preempted or blocked, push it back to the ready queue
        if (status == OS_RUN_PREEMPTED) {
            ready_queue_push(ready_queue, process);
        } else if (status == OS_RUN_BLOCKED) {
            update_quantums(process, quantums);
            os_start_io(process);
        }

    }

    return NULL;
}

void update_quantums(process_t *process, uint64_t *quantums) {

    // If quantum is 0, initialize it with the recorded burst of the first process
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        if (quantums[i] == 0) {
            quantums[i] = process->burst_length * CONSTANTS[i];
        } else {
            // Initialize alpha with ALPHA_LOW
            float alpha = ALPHA_LOW;
            // Give a higher weight to the new recorded burst if it is higher than the current quantum
            if (process->burst_length > quantums[i]) {
                alpha = ALPHA_HIGH;
            }
            // Update quantum with the new recorded burst
            quantums[i] = (1 - alpha) * quantums[i] + alpha * process->burst_length * CONSTANTS[i];
        }
    }

    // Reset the burst length of the process
    process->burst_length = 0;
}

void update_priority_level(process_t *process, int status) {
    switch (status) {
        case OS_RUN_PREEMPTED:
            // Process was preempted, demote priority level
            process->priority_level = max(process->priority_level - 1, LOW_PRIORITY_LEVEL);
            break;
        case OS_RUN_BLOCKED:
            // Process was blocked, promote priority level
            process->priority_level = min(process->priority_level + 1, HIGH_PRIORITY_LEVEL);
            break;
        default:
            // Process completed its burst, reset priority level to base
            process->priority_level = BASE_PRIORITY_LEVEL;
            break;

    }
}

worker_t *worker_create(int core, ready_queue_t *ready_queue) {
    // Allocate memory for the worker
    worker_t *worker = malloc(sizeof(worker_t));

    // Initialize the worker
    worker->core = core;
    worker->ready_queue = ready_queue;

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

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

