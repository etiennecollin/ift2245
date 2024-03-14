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
        pthread_mutex_init(&queue->mutex[i], NULL);
        queue->size[i] = 0;
    }
    pthread_mutex_init(&queue->global_mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void ready_queue_destroy(ready_queue_t *queue) {
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        pthread_mutex_lock(&queue->mutex[i]);
        node_t *current = queue->head[i];
        while (current != NULL) {
            node_t *next = current->next;
            free(current);
            current = next;
        }
        pthread_mutex_unlock(&queue->mutex[i]);
        pthread_mutex_destroy(&queue->mutex[i]);
    }
    pthread_cond_destroy(&queue->cond);
}

void ready_queue_push(ready_queue_t *queue, process_t *process) {
    int priority = DEFAULT_PRIORITY_LEVEL + 1;
    if (process != NULL) {
        priority = process->priority_level;
    }

    // Lock the queue
    pthread_mutex_lock(&queue->mutex[priority]);

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
    pthread_mutex_unlock(&queue->mutex[priority]);
    pthread_cond_signal(&queue->cond);
}

process_t *ready_queue_pop(ready_queue_t *queue) {
    int priority = -1;
    // Pop the highest priority process
    while (1) {
        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            if (queue->size[i] > 0) {
                pthread_mutex_lock(&queue->mutex[i]);
                priority = i;
                break;
            }
        }
        if (priority == -1) {
            pthread_mutex_lock(&queue->global_mutex);
            pthread_cond_wait(&queue->cond, &queue->global_mutex);
            pthread_mutex_unlock(&queue->global_mutex);
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
    pthread_mutex_unlock(&queue->mutex[priority]);

    // Return the process
    return process;
}


void ready_queue_remove(ready_queue_t *queue, process_t *process) {
    if (process == NULL)
        return;

    int priority = process->priority_level;

    // Lock the queue
    pthread_mutex_lock(&queue->mutex[priority]);

    // Traverse the queue to find and remove the process
    node_t *current = queue->head[priority];
    node_t *prev = NULL;
    while (current != NULL) {
        if (current->process == process) {
            if (prev == NULL) { // Process is at the head of the queue
                queue->head[priority] = current->next;
                if (queue->tail[priority] == current) { // Process is the only one in the queue
                    queue->tail[priority] = NULL;
                }
            } else {
                prev->next = current->next;
                if (queue->tail[priority] == current) { // Process is at the tail of the queue
                    queue->tail[priority] = prev;
                }
            }
            free(current); // Free the node
            queue->size[priority]--; // Decrement the size
            break; // Exit the loop after removing the process
        }
        prev = current;
        current = current->next;
    }

    // Unlock the queue
    pthread_mutex_unlock(&queue->mutex[priority]);
}


size_t ready_queue_size(ready_queue_t *queue) {
    // Lock all queues
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        pthread_mutex_lock(&queue->mutex[i]);
    }

    // Calculate the total size
    size_t size = 0;
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        size += queue->size[i];
    }

    // Unlock all queues
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        pthread_mutex_unlock(&queue->mutex[i]);
    }
    return size;
}