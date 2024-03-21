#include "ready_queue.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
    pthread_mutex_destroy(&queue->read_max_queue_mutex);
    pthread_cond_destroy(&queue->cond);
}

void remove_from_quantum(ready_queue_t *queue, process_t *process) {
    if (process == NULL || !process->found_burst) {
        return;
    }

    uint64_t quantum = queue->quantum;
    int counter = queue->quantum_counter;
    queue->quantum = (quantum * counter - process->burst_length) / (counter - 1);
    queue->quantum_counter--;
}

void add_to_quantum(ready_queue_t *queue, process_t *process) {
    if (process == NULL || process->found_burst) {
        return;
    }

    uint64_t quantum = queue->quantum;
    int counter = queue->quantum_counter;
    queue->quantum = (quantum * counter + process->burst_length) / (counter + 1);
    queue->quantum_counter++;
}


void ready_queue_push(ready_queue_t *queue, process_t *process) {
    int priority = DEFAULT_PRIORITY_LEVEL + 1;
    if (process != NULL) {
        priority = process->priority_level;
    }

    // Lock the queue
    pthread_mutex_lock(&queue->queue_mutex[priority]);

    // Allocate a new node
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

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

    // Free the node
    free(head);

    // Unlock the queue
    pthread_mutex_unlock(&queue->queue_mutex[priority]);

    // Return the process
    return process;
}


int ready_queue_remove(ready_queue_t *queue, process_t *process) {
    if (process == NULL)
        return 0;

    int priority = process->priority_level;

    // Lock the queue
    pthread_mutex_lock(&queue->queue_mutex[priority]);

    // Traverse the queue to find and remove the process
    node_t *prev = NULL;
    node_t *current = queue->tail[priority];
    int was_found = 0;
    while (current != NULL) {
        if (current->process == process) {
            was_found = 1;
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
    }

    // Unlock the queue
    pthread_mutex_unlock(&queue->queue_mutex[priority]);

    return was_found;
}


size_t ready_queue_size(ready_queue_t *queue) {
    // Lock all queues
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        pthread_mutex_lock(&queue->queue_mutex[i]);
    }

    // Calculate the total size
    size_t size = 0;
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        size += queue->size[i];
    }

    // Unlock all queues
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        pthread_mutex_unlock(&queue->queue_mutex[i]);
    }
    return size;
}