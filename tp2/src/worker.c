#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "os.h"

#include <unistd.h>  // For sleep function

// Time in seconds to wait before checking if the priority level of a process needs to be increased
#define BOOST_TIME 3

// Maximum value of the "micro quantum"
#define MICRO_QUANTUM_MAX 120

// Constant factor multiplying the average quantum of the queue
// The higher the priority level, the lower the constant factor
// The order of the constants goes from the highest priority level to the lowest
// i.e. const float CONSTANTS[] = {HIGHEST_PRIORITY_CONSTANT, ..., LOWEST_PRIORITY_CONSTANT};
const float CONSTANTS[] = {0, 0.3, 0.5, 0.8};

void *worker_run(void *user_data) {
    worker_t *worker = (worker_t *) user_data;
    ready_queue_t *ready_queue = worker->ready_queue;

    int mutex_status = -1;
    process_t *process = NULL;
    uint64_t start_time;
    uint64_t quantum;
    while (1) {
        // Pop a process from the ready queue
        process = ready_queue_pop(ready_queue);

        // This mutex makes sure that only one worker can preempt
        // between micro quantums and get the new max priority
        // level process.
        if (mutex_status == 0) {
            pthread_mutex_unlock(&ready_queue->read_max_queue_mutex);
            mutex_status = -1;
        }

        // No more processes (poison pill)
        if (process == NULL) break;

        // Get the quantum for the current priority level
        quantum = get_quantum(ready_queue, process->priority_level);

        // Run the process
        pthread_mutex_lock(&process->mutex);
        if (process->priority_level != MAX_PRIORITY_LEVEL) {
            // Run the process, but split the quantum into micro quantums
            // to check if the process should be preempted by a higher priority process.
            // This works because the OS does not impose a context switch delay
            // if a process that is run is the same as the one that was previously run.
            uint64_t micro_quantum = min(MICRO_QUANTUM_MAX, quantum);
            for (uint64_t i = 0; i < quantum; i += micro_quantum) {
                // Set the arrival time of the process
                if (!process->found_burst) {
                    start_time = os_time();
                }

                // Run the process for a micro quantum
                process->status = os_run_process(process, worker->core, micro_quantum);

                // Update the burst length of the process
                if (!process->found_burst) {
                    process->burst_length += os_time() - start_time;
                }

                // Check if the process should be preempted.
                mutex_status = pthread_mutex_trylock(&ready_queue->read_max_queue_mutex);
                if (mutex_status == 0) {
                    if (ready_queue->size[MAX_PRIORITY_LEVEL] == 1) {
                        // Only one process in the max priority level queue
                        // Therefore, preempt the current process and do not
                        // allow other workers to try to steal it by preempting.
                        break;
                    } else if (ready_queue->size[MAX_PRIORITY_LEVEL] > 1) {
                        // More than one process in the max priority level queue
                        // Therefore, preempt the current process and allow
                        // the other workers to try to preempt.
                        pthread_mutex_unlock(&ready_queue->read_max_queue_mutex);
                        mutex_status = -1;
                        break;
                    } else {
                        // No process in the max priority level queue
                        pthread_mutex_unlock(&ready_queue->read_max_queue_mutex);
                        mutex_status = -1;
                    }
                }

                // In any case, the loop will break if the process is blocked or done.
                if (process->status == OS_RUN_BLOCKED || process->status == OS_RUN_DONE) {
                    break;
                }
            }
        } else {
            // Set the arrival time of the process
            if (!process->found_burst) {
                start_time = os_time();
            }

            // Run the process for the quantum
            process->status = os_run_process(process, worker->core, quantum);

            // Update the burst length of the process
            if (!process->found_burst) {
                process->burst_length += os_time() - start_time;
            }
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
                // Include the process in the quantum average
                add_to_quantum_average(ready_queue, process);
                process->found_burst = 1;

                // Update the io length of the process
                if (!process->found_io) {
                    start_time = os_time();
                }

                // Start the IO of the process
                os_start_io(process);

                // Update the IO length of the process
                if (!process->found_io) {
                    process->io_length = os_time() - start_time;
                    process->found_burst = 1;
                }
                break;
            case OS_RUN_DONE:
                remove_from_quantum_average(ready_queue, process);
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
        return MAX_PRIORITY_LEVEL_QUANTUM;
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
        // By default, the new level is the maximum priority level + 1
        int new_level = MAX_PRIORITY_LEVEL + 1;

        // If the process is blocked, then the new level is the maximum priority level
        if (process->status == OS_RUN_BLOCKED) {
            new_level = MAX_PRIORITY_LEVEL;
        }

        // Check if the priority level of the process is higher than the new level
        if (process->priority_level > new_level) {
            process->priority_level = new_level;

            // Remove the process from the ready queue if it is there and push it back to give it the right queue.
            // If it is not in the queue, then the process will be automatically added back to the ready queue
            // by the worker or the OS.
            int was_removed = ready_queue_remove(ready_queue, process);
            if (was_removed) {
                // Add the process to the right priority level ready queue
                ready_queue_push(ready_queue, process);
            }
        }
    }
    process->waiting_boost = 0;
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

            if (BOOST_TIME != 0 && NUM_PRIORITY_LEVELS > 2 && !process->waiting_boost) {
                process->waiting_boost = 1;

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
            pthread_mutex_unlock(&process->mutex);
            break;
        case OS_RUN_BLOCKED:
            // Process was blocked, promote priority level
            pthread_mutex_lock(&process->mutex);
            process->priority_level = MAX_PRIORITY_LEVEL;
            pthread_mutex_unlock(&process->mutex);
            break;
        case OS_RUN_DONE:
            // Process is done, reset priority level
            pthread_mutex_lock(&process->mutex);
            process->priority_level = MAX_PRIORITY_LEVEL;
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

