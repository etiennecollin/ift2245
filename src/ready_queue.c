#include "ready_queue.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "os.h"

void ready_queue_init(ready_queue_t *queue) {
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->size = 0;
}

void ready_queue_destroy(ready_queue_t *queue) {
    node_t *current = queue->head;
    while (current != NULL) {
        node_t *next = current->next;
        free(current);
        current = next;
    }
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

void ready_queue_push(ready_queue_t *queue, process_t *process) {
    // Lock the queue
    pthread_mutex_lock(&queue->mutex);

    // Allocate a new node
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Initialize the new node
    new_node->process = process;
    new_node->next = NULL;
    if (queue->tail != NULL) {
        queue->tail->next = new_node;
    } else {
        queue->head = new_node;
    }
    queue->tail = new_node;
    queue->size++;

    // Signal the condition variable
    // and unlock the queue
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

process_t *ready_queue_pop(ready_queue_t *queue) {
    // Lock the queue
    pthread_mutex_lock(&queue->mutex);

    // Wait for a process to be added
    while (queue->head == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    // Save the process of the node
    node_t *head = queue->head;
    process_t *process = head->process;

    // Update the queue's head and tail
    queue->head = head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    // Free the node
    free(head);
    queue->size--;

    // Unlock the queue
    pthread_mutex_unlock(&queue->mutex);

    // Return the process
    return process;
}

size_t ready_queue_size(ready_queue_t *queue) {
    // Lock the queue and get the size
    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;

    // Unlock the queue and return the size
    pthread_mutex_unlock(&queue->mutex);
    return size;
}