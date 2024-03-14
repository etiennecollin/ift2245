#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

#include <unistd.h>  // For sleep function


const uint64_t INITIAL_QUANTUM = 0; // Initial quantum for all priority levels

// Weight of the new recorded burst in the quantum average calculation
// if the new recorded burst is lower than the current quantum
const float ALPHA_LOW = 0.4;
// Weight of the new recorded burst in the quantum average calculation
// if the new recorded burst is higher than the current quantum
const float ALPHA_HIGH = 0.1;

// Constant factor multiplying the new recorded burst
// The higher the priority level, the lower the constant factor
// The order of the constants is goes from the highest priority level to the lowest
// i.e. const float CONSTANTS[] = {HIGHEST_PRIORITY_CONSTANT, ..., LOWEST_PRIORITY_CONSTANT};
const float CONSTANTS[] = {0.6, 1.1, 1.2};
//const float CONSTANTS[] = {0.1,0.4,0.6};

// Time in seconds to wait before checking if the priority level of a process needs to be increased
const int BOOST_TIME = 5;


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
    while (1) {
        // Pop a process from the ready queue
        process = ready_queue_pop(ready_queue);

        // No more processes (poison pill)
        if (process == NULL) break;

        // Get the quantum for the current priority level
        quantum = quantums[process->priority_level];

        // Set the arrival time of the process
        uint64_t start_time = os_time();

        // Run the process
        pthread_mutex_lock(&process->mutex);
        process->status = os_run_process(process, worker->core, quantum);
        pthread_mutex_unlock(&process->mutex);

        // Update the burst length of the process
        process->burst_length += os_time() - start_time;

        // Adjust priority level based on process behavior
//        update_priority_level(process, ready_queue);

        // If the process was preempted or blocked, push it back to the ready queue
        pthread_mutex_lock(&process->mutex);
        switch (process->status) {
            case OS_RUN_PREEMPTED:
                ready_queue_push(ready_queue, process);
                break;
            case OS_RUN_BLOCKED:
                update_quantums(process, quantums);
                os_start_io(process);
                break;
            case OS_RUN_DONE:
                process->status = 1;
                break;

        }
        pthread_mutex_unlock(&process->mutex);
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

void *priority_monitor_thread(void *thread_data) {
    // Extract process and ready_queue from the argument
    process_t *process = ((struct queue_process_data *) thread_data)->process;
    ready_queue_t *ready_queue = ((struct queue_process_data *) thread_data)->ready_queue;

    int current_priority = process->priority_level;
    sleep(BOOST_TIME);  // Sleep for 5 seconds

    // Check if the priority is still the same or decreased after 5 seconds
    pthread_mutex_lock(&process->mutex);
    if (process->priority_level <= current_priority && !process->status) {
        // Increase priority back by 1
        process->priority_level = max(process->priority_level - 1, MAX_PRIORITY_LEVEL);

        // Check if the process needs to be moved to a different queue
        if (process->priority_level != current_priority) {
            // Remove the process from the ready queue
            ready_queue_remove(ready_queue, process);
            // Add the process to the new ready queue
            ready_queue_push(ready_queue, process);
        }
    }
    pthread_mutex_unlock(&process->mutex);

    free(thread_data);
    return NULL;
}

void update_priority_level(process_t *process, ready_queue_t *ready_queue) {
    switch (process->status) {
        case OS_RUN_PREEMPTED:
            // Process was preempted, demote priority level
            pthread_mutex_lock(&process->mutex);
            process->priority_level = min(process->priority_level + 1, MIN_PRIORITY_LEVEL);
            pthread_mutex_unlock(&process->mutex);

            if (BOOST_TIME != 0) {
                // Create an instance of the structure and populate it with process and ready_queue data
                // Allocate memory for the struct
                queue_process_data_t *data = malloc(sizeof(queue_process_data_t));
                data->process = process;
                data->ready_queue = ready_queue;

                // Start a new thread to monitor the priority change
                pthread_t monitor_thread;
                pthread_create(&monitor_thread, NULL, priority_monitor_thread, data);
                pthread_detach(monitor_thread);  // Detach the thread to avoid memory leak
            }
            break;
        case OS_RUN_BLOCKED:
            // Process was blocked, promote priority level
            pthread_mutex_lock(&process->mutex);
            process->priority_level = max(process->priority_level - 1, MAX_PRIORITY_LEVEL);
            pthread_mutex_unlock(&process->mutex);
            break;
        default:
            // Process completed its burst, reset priority level to base
            pthread_mutex_lock(&process->mutex);
            process->priority_level = DEFAULT_PRIORITY_LEVEL;
            pthread_mutex_unlock(&process->mutex);
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

