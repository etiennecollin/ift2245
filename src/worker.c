#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

uint64_t quantum = 0; // In milliseconds

// Weight of the new recorded burst in the quantum average calculation
const float alpha_low = 0.4; // If the new recorded burst is lower than the current quantum
const float alpha_high = 0.15; // If the new recorded burst is higher than the current quantum
const float gamma = 1.2; // Constant factor multiplying the new recorded burst

// Update quantum with a dynamic leaky filter average given the recorded burst of a process
void *update_quantum(process_t *process) {
    // If quantum is 0, initialize it with the recorded burst of the first process
    if (quantum == 0) {
        quantum = process->burst_length * gamma;
    } else {
        // Initialize alpha with alpha_low
        float alpha = alpha_low;
        // Give a higher weight to the new recorded burst if it is higher than the current quantum
        if (process->burst_length > quantum) {
            alpha = alpha_high;
        }
        // Update quantum with the new recorded burst
        quantum = (1 - alpha) * quantum + alpha * process->burst_length * gamma;
    }

    // Reset the burst length of the process
    process->burst_length = 0;
}

void *worker_run(void *user_data) {

    worker_t *worker = (worker_t *) user_data;
    ready_queue_t *ready_queue = worker->ready_queue;

    process_t *process = NULL;

    for (;;) {
        process = ready_queue_pop(ready_queue);
        if (process == NULL) break; // No more processes (poison pill)

        // Set the arrival time of the process
        uint64_t start_time = os_time();

        // Run the process
        int status = os_run_process(process, worker->core, quantum);

        process->burst_length += os_time() - start_time;

        // If the process was preempted or blocked, push it back to the ready queue
        if (status == OS_RUN_PREEMPTED) {
            ready_queue_push(worker->ready_queue, process);
        } else if (status == OS_RUN_BLOCKED) {
            update_quantum(process);
            os_start_io(process);
        }

    }


    return NULL;
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
