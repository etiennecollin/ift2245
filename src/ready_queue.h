#ifndef TP2_READY_QUEUE_H
#define TP2_READY_QUEUE_H

#include <stdint.h>
#include <pthread.h>

#include "process.h"

typedef struct ready_queue ready_queue_t;

struct ready_queue {
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    // TODO: Add fields here
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
 * Cette fonction retourne la taille de la file d'attente.
 *
 * @param queue le ready queue
 *
 * @return la taille de la file d'attente
 */
size_t ready_queue_size(ready_queue_t *queue);


#endif
