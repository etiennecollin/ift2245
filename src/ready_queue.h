#ifndef TP2_READY_QUEUE_H
#define TP2_READY_QUEUE_H

#include <stdint.h>
#include <pthread.h>

#include "process.h"

#define INITIAL_QUANTUM 125

typedef struct ready_queue ready_queue_t;
typedef struct node node_t;

struct ready_queue {
    pthread_cond_t cond;
    pthread_mutex_t useless_mutex;

    pthread_mutex_t queue_mutex[NUM_PRIORITY_LEVELS];
    pthread_mutex_t read_max_queue_mutex;
    node_t *head[NUM_PRIORITY_LEVELS];
    node_t *tail[NUM_PRIORITY_LEVELS];
    size_t size[NUM_PRIORITY_LEVELS];

    uint64_t quantum;
    int quantum_counter;
};

struct node {
    process_t *process;
    struct node *next;
    struct node *prev;
};

/**
 * Cette fonction initialise le ready queue.
 *
 * @param queue le ready queue à initialiser
 */
void ready_queue_init(ready_queue_t *queue);

/**
 * Cette fonction détruit le ready queue.
 *
 * @param queue le ready queue à détruire
 */
void ready_queue_destroy(ready_queue_t *queue);

/**
 * Cette fonction ajoute un processus à la file d'attente. Un coeur
 * sera réveillé si un processus a été ajouté à une file d'attente.
 *
 * Si le processus ajouté est `NULL`, c'est un `poison pill` qui est
 * ajouté à la file d'attente. Cela signifie que le coeur qui retire
 * ce `poison pill` doit se terminer. Les `poison pills` seront ajoutés
 * autant a la fin de l'exécution du programme, une fois que tous les
 * processus ont été ajoutés à la file d'attente.
 *
 * @param queue le ready queue
 * @param process le processus à ajouter
 */
void ready_queue_push(ready_queue_t *queue, process_t *process);

/**
 * Cette fonction retire un processus de la file d'attente. Si la file
 * d'attente est vide, le coeur appelant sera bloqué jusqu'à ce qu'un
 * processus soit ajouté à la file d'attente.
 *
 * Si le processus retiré est `NULL`, c'est un `poison pill` qui a été
 * retiré de la file d'attente. Cela signifie que le coeur doit se
 * terminer.
 *
 * @param queue le ready queue
 *
 * @return le processus retiré
 */
process_t *ready_queue_pop(ready_queue_t *queue);


/**
 *  This function removes a process from its queue.
 *
 * @param queue the ready queue
 * @param process the removed process
 *
 * @return 0 if the process was removed, 1 otherwise
 */
int ready_queue_remove(ready_queue_t *queue, process_t *process);

/**
 * Cette fonction retourne la taille de la file d'attente.
 *
 * @param queue le ready queue
 *
 * @return la taille de la file d'attente
 */
size_t ready_queue_size(ready_queue_t *queue);


void add_to_quantum(ready_queue_t *queue, process_t *process);

void remove_from_quantum(ready_queue_t *queue, process_t *process);

#endif
