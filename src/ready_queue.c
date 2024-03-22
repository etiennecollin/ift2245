#include "ready_queue.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "os.h"

void ready_queue_init(ready_queue_t *queue) {
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        queue->head[i] = NULL;
        queue->tail[i] = NULL;
        queue->size[i] = 0;
        pthread_mutex_init(&queue->queue_mutex[i], NULL);
    }
    queue->quantum = INITIAL_QUANTUM;
    queue->quantum_counter = 0;
    pthread_mutex_init(&queue->useless_mutex, NULL);
    pthread_mutex_init(&queue->quantum_mutex, NULL);
    pthread_mutex_init(&queue->read_max_queue_mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void ready_queue_destroy(ready_queue_t *queue) {
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        pthread_mutex_lock(&queue->queue_mutex[i]);
        node_t *current = queue->head[i];
        while (current != NULL) {
            node_t *next = current->next;
            free(current);
            current = next;
        }
        pthread_mutex_unlock(&queue->queue_mutex[i]);
        pthread_mutex_destroy(&queue->queue_mutex[i]);
    }
    pthread_mutex_destroy(&queue->useless_mutex);
    pthread_mutex_destroy(&queue->quantum_mutex);
    pthread_mutex_destroy(&queue->read_max_queue_mutex);
    pthread_cond_destroy(&queue->cond);
}

void remove_from_quantum_average(ready_queue_t *queue, process_t *process) {
    if (process == NULL || !process->found_burst) {
        return;
    }

    pthread_mutex_lock(&queue->quantum_mutex);
    uint64_t quantum = queue->quantum;
    int counter = queue->quantum_counter;
    queue->quantum = (quantum * counter - process->burst_length) / (counter - 1);
    queue->quantum_counter--;
    pthread_mutex_unlock(&queue->quantum_mutex);
}

void add_to_quantum_average(ready_queue_t *queue, process_t *process) {
    if (process == NULL || process->found_burst) {
        return;
    }

    pthread_mutex_lock(&queue->quantum_mutex);
    uint64_t quantum = queue->quantum;
    int counter = queue->quantum_counter;
    queue->quantum = (quantum * counter + process->burst_length) / (counter + 1);
    queue->quantum_counter++;
    pthread_mutex_unlock(&queue->quantum_mutex);
}


void ready_queue_push(ready_queue_t *queue, process_t *process) {
    int priority = DEFAULT_PRIORITY_LEVEL + 1;
    if (process != NULL) {
        priority = process->priority_level;
    }

    // Allocate a new node
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Lock the queue
    pthread_mutex_lock(&queue->queue_mutex[priority]);

    // Initialize the new node
    new_node->process = process;
    new_node->next = NULL;
    new_node->prev = NULL;
    if (process == NULL || priority == MAX_PRIORITY_LEVEL) {
        if (queue->tail[priority] != NULL) {
            new_node->next = queue->tail[priority];
            queue->tail[priority]->prev = new_node;
        } else {
            queue->head[priority] = new_node;
        }
        queue->tail[priority] = new_node;
    } else if (process->already_executed) {
        // Add process to head
        if (queue->head[priority] != NULL) {
            queue->head[priority]->next = new_node;
            new_node->prev = queue->head[priority];
        } else {
            queue->tail[priority] = new_node;
        }
        queue->head[priority] = new_node;
    } else {
        // Add the process to the right position based on its burst length + io length
        // Find the right position
        node_t *prev = NULL;
        node_t *current = queue->tail[priority];
        while (current != NULL) {
            if (current->process->burst_length + current->process->io_length <
                process->burst_length + process->io_length) {
                break;
            }
            prev = current;
            current = current->next;
        }

        // Link the neighbors of the new node
        new_node->next = current;
        new_node->prev = prev;
        if (prev != NULL) {
            prev->next = new_node;
        } else {
            queue->tail[priority] = new_node;
        }
        if (current != NULL) {
            current->prev = new_node;
        } else {
            queue->head[priority] = new_node;
        }
    }
    queue->size[priority]++;

    // Signal the condition variable and unlock the queue
    pthread_mutex_unlock(&queue->queue_mutex[priority]);
    pthread_cond_signal(&queue->cond);
}

process_t *ready_queue_pop(ready_queue_t *queue) {
    int priority = -1;
    // Pop the highest priority process
    while (1) {
        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            if (queue->size[i] > 0) {
                pthread_mutex_lock(&queue->queue_mutex[i]);
                if (queue->size[i] > 0) {
                    priority = i;
                    break;
                } else {
                    pthread_mutex_unlock(&queue->queue_mutex[i]);
                    continue;
                }
            }
        }
        if (priority == -1) {
            pthread_mutex_lock(&queue->useless_mutex);
            pthread_cond_wait(&queue->cond, &queue->useless_mutex);
            pthread_mutex_unlock(&queue->useless_mutex);
        } else {
            break;
        }
    }

    // Decrement the size of the queue
    queue->size[priority]--;

    // Save the process of the node
    node_t *head = queue->head[priority];

    // Verify if the head is null
    if (head == NULL) {
        return NULL;
    }

    // Get the process
    process_t *process = head->process;

    // Update the queue's head and tail
    queue->head[priority] = queue->head[priority]->prev;
    if (queue->head[priority] == NULL) {
        queue->tail[priority] = NULL;
    } else {
        queue->head[priority]->next = NULL;
    }

    // Unlock the queue
    pthread_mutex_unlock(&queue->queue_mutex[priority]);

    // Free the node
    free(head);

    // Return the process
    return process;
}


int ready_queue_remove(ready_queue_t *queue, process_t *process) {
    if (process == NULL)
        return 0;

    int priority = process->priority_level;

    // Allocate memory for the struct and set the data
    remove_parallel_data_t *data = malloc(sizeof(remove_parallel_data_t));
    data->process = process;
    data->queue = queue;

    // Create the left and right threads
    void *found_left;
    void *found_right;
    pthread_t left_thread;
    pthread_t right_thread;

    // Lock the queue
    pthread_mutex_lock(&queue->queue_mutex[priority]);

    // Start a new thread to monitor the priority change
    pthread_create(&left_thread, NULL, remove_left_thread, data);
    pthread_create(&right_thread, NULL, remove_right_thread, data);

    // Wait for the threads to finish
    pthread_join(left_thread, &found_left);
    pthread_join(right_thread, &found_right);

    // Unlock the queue
    pthread_mutex_unlock(&queue->queue_mutex[priority]);
    int found = *((int *) found_left) || *((int *) found_right);

    // Free the data
    free(data);
    free(found_left);
    free(found_right);

    return found;
}

void *remove_left_thread(void *thread_data) {
    // Extract process and queue from the argument
    struct remove_parallel_data *data = (struct remove_parallel_data *) thread_data;
    process_t *process = data->process;
    ready_queue_t *queue = data->queue;

    int priority = process->priority_level;
    int moves = floor(queue->size[priority] / 2);

    // Traverse the queue to find and remove the process
    node_t *prev = NULL;
    node_t *current = queue->tail[priority];

    void *was_found = malloc(sizeof(int));
    *((int *) was_found) = 0;

    int position = 1;
    while (current != NULL && position < moves && !data->stop) {
        if (current->process == process) {
            // Set the stop flag and the was_found flag
            data->stop = 1;
            *((int *) was_found) = 1;

            // Remove the node from the queue
            if (prev == NULL) {
                queue->tail[priority] = current->next;
            } else {
                prev->next = current->next;
            }

            if (current->next == NULL) {
                queue->head[priority] = current->prev;
            } else {
                current->next->prev = prev;
            }

            // Free the node and decrement the size
            free(current);
            queue->size[priority]--;
            break;
        }
        prev = current;
        current = current->next;
        position++;
    }

    return was_found;
}

void *remove_right_thread(void *thread_data) {
    // Extract process and queue from the argument
    struct remove_parallel_data *data = (struct remove_parallel_data *) thread_data;
    process_t *process = data->process;
    ready_queue_t *queue = data->queue;

    int priority = process->priority_level;
    int moves = ceil(queue->size[priority] / 2);

    // Traverse the queue to find and remove the process
    node_t *prev = NULL;
    node_t *current = queue->head[priority];

    void *was_found = malloc(sizeof(int));
    *((int *) was_found) = 0;

    int position = 0;
    while (current != NULL && position < moves && !data->stop) {
        if (current->process == process) {
            // Set the stop flag and the was_found flag
            data->stop = 1;
            *((int *) was_found) = 1;

            // Remove the node from the queue
            if (prev == NULL) {
                queue->head[priority] = current->prev;
            } else {
                prev->prev = current->prev;
            }

            if (current->prev == NULL) {
                queue->tail[priority] = current->next;
            } else {
                current->prev->next = prev;
            }

            // Free the node and decrement the size
            free(current);
            queue->size[priority]--;
            break;
        }
        prev = current;
        current = current->prev;
        position++;
    }

    return was_found;
}


size_t ready_queue_size(ready_queue_t *queue) {
    // Calculate the total size
    // No need to lock the queues here since we will not
    // have 100% accurate size anyway because of the time it
    // takes to lock the queues.
    size_t size = 0;
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        size += queue->size[i];
    }

    return size;
}