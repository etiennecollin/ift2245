#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

#include <unistd.h>  // For sleep function

// Time in seconds to wait before checking if the priority level of a process needs to be increased
const int BOOST_TIME = 3;


// Constant factor multiplying the new recorded burst
// The higher the priority level, the lower the constant factor
// The order of the constants is goes from the highest priority level to the lowest
// i.e. const float CONSTANTS[] = {HIGHEST_PRIORITY_CONSTANT, ..., LOWEST_PRIORITY_CONSTANT};
const float CONSTANTS[] = {0, 0.5, 0.7, 1.0};

void *worker_run(void *user_data) {
    worker_t *worker = (worker_t *) user_data;
    ready_queue_t *ready_queue = worker->ready_queue;

    process_t *process = NULL;
    uint64_t start_time;
    uint64_t quantum;
    while (1) {
        // Pop a process from the ready queue
        process = ready_queue_pop(ready_queue);

        // No more processes (poison pill)
        if (process == NULL) break;

        // Get the quantum for the current priority level
        quantum = get_quantum(ready_queue, process->priority_level);

        // Run the process
        pthread_mutex_lock(&process->mutex);
        if (process->priority_level != MAX_PRIORITY_LEVEL) {
            uint64_t quantum_spent = 0;
            uint64_t micro_quantum = min(100, quantum);
            while (1) {
                // Set the arrival time of the process
                start_time = os_time();

                // Run the process for a micro quantum
                process->status = os_run_process(process, worker->core, micro_quantum);

                // Update the burst length of the process
                if (!process->found_burst) {
                    process->burst_length += os_time() - start_time;
                }

                quantum_spent += micro_quantum;

                // Check if the process should be preempted
                pthread_mutex_lock(&ready_queue->queue_mutex[process->priority_level]);
                if (ready_queue->size[MAX_PRIORITY_LEVEL] > 0
                    || process->status == OS_RUN_BLOCKED
                    || process->status == OS_RUN_DONE
                    || quantum_spent >= quantum) {
                    pthread_mutex_unlock(&ready_queue->queue_mutex[process->priority_level]);
                    break;
                }
                pthread_mutex_unlock(&ready_queue->queue_mutex[process->priority_level]);
            }
        } else {
            process->status = os_run_process(process, worker->core, quantum);
        }
        pthread_mutex_unlock(&process->mutex);

        // Adjust priority level based on process behavior
        update_priority_level(process, ready_queue);

        // If the process was preempted or blocked, push it back to the ready queue
        pthread_mutex_lock(&process->mutex);
        switch (process->status) {
            case OS_RUN_PREEMPTED:
                ready_queue_push(ready_queue, process);
                break;
            case OS_RUN_BLOCKED:
                add_to_quantum(ready_queue, process);
                process->found_burst = 1;

                start_time = os_time();
                os_start_io(process);

                // Update the io length of the process
                if (!process->found_io) {
                    process->io_length = os_time() - start_time;
                    process->found_burst = 1;
                }
                break;
            case OS_RUN_DONE:
                remove_from_quantum(ready_queue, process);
                process->found_burst = 0;
                process->burst_length = 0;
                break;
        }
        pthread_mutex_unlock(&process->mutex);
    }
    return NULL;
}

uint64_t get_quantum(ready_queue_t *queue, int priority) {
    if (priority == MAX_PRIORITY_LEVEL) {
        return 1;
    }

    int quantum = queue->quantum * CONSTANTS[priority];
    if (quantum == 0) {
        quantum = INITIAL_QUANTUM;
    }

    return quantum;
}

void *priority_monitor_thread(void *thread_data) {
    // Extract process and ready_queue from the argument
    process_t *process = ((struct queue_process_data *) thread_data)->process;
    ready_queue_t *ready_queue = ((struct queue_process_data *) thread_data)->ready_queue;

    // Sleep for a while
    sleep(BOOST_TIME);

    // Check if the priority is still the same or decreased during the sleep
    pthread_mutex_lock(&process->mutex);
    if (process->status != OS_RUN_DONE) {
        // Increase priority back to what it was
        process->priority_level = MAX_PRIORITY_LEVEL + 1;

        // Remove the process from the ready queue if it is there
        // If it is not in the queue, then the process will be added back to the ready queue
        // by the worker or OS automatically.
        int was_removed = ready_queue_remove(ready_queue, process);
        if (was_removed) {
            // Add the process to the right priority level ready queue
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

            if (BOOST_TIME != 0 && NUM_PRIORITY_LEVELS > 2) {
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
            if (NUM_PRIORITY_LEVELS > 2) {
                pthread_mutex_lock(&process->mutex);
                process->priority_level = max(process->priority_level - 1, MAX_PRIORITY_LEVEL + 1);
                pthread_mutex_unlock(&process->mutex);
            }
            break;
        case OS_RUN_DONE:
            // Process is done, reset priority level
            pthread_mutex_lock(&process->mutex);
            process->priority_level = MIN_PRIORITY_LEVEL;
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

