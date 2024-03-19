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
    if (queue->tail[priority] != NULL) {
        queue->tail[priority]->next = new_node;
    } else {
        queue->head[priority] = new_node;
    }
    queue->tail[priority] = new_node;
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
                priority = i;
                break;
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

    // Save the process of the node
    node_t *head = queue->head[priority];

    // Decrement the size of the queue
    queue->size[priority]--;

    // Verify if the head is null
    if (head == NULL) {
        return NULL;
    }

    // Get the process
    process_t *process = head->process;

    // Update the queue's head and tail
    queue->head[priority] = head->next;
    if (queue->head[priority] == NULL) {
        queue->tail[priority] = NULL;
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
    node_t *current = queue->head[priority];
    node_t *prev = NULL;
    int was_found = 0;
    while (current != NULL) {
        if (current->process == process) {
            was_found = 1;
            // Process is at the head of the queue
            if (prev == NULL) {
                // Update the head of the queue
                queue->head[priority] = current->next;

                // If process is the only one in the queue
                // update the tail of the queue
                if (queue->tail[priority] == current) {
                    queue->tail[priority] = NULL;
                }
            } else {
                // Update the next of the previous node
                prev->next = current->next;

                // If process is at the tail of the queue
                // update the tail of the queue
                if (queue->tail[priority] == current) {
                    queue->tail[priority] = prev;
                }
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