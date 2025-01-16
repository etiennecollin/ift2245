#ifndef TP2_OS_H
#define TP2_OS_H

#include <stdint.h>

#include "process.h"

#define OS_CORES 2
#define OS_CONTEXT_SWITCH_DURATION 20

#define OS_RUN_DONE 0
#define OS_RUN_BLOCKED 1
#define OS_RUN_PREEMPTED 2

/**
 * Cette fonction met un processus en cours d'exécution sur un coeur et bloque jusqu'à ce que le processus
 * ait terminé son exécution ou ait été préempté.
 *
 * Context Switch: Si le processus en cours d'exécution est le différent de celui passé en paramètre, un
 *                 context switch va être effectué qui va prendre OS_CONTEXT_SWITCH_DURATION ms.
 *
 * Retour:
 * - OS_RUN_DONE: le processus en cours d'exécution a terminé son exécution
 * - OS_RUN_BLOCKED: le processus en cours d'exécution a terminé son burst et doit effectuer une opération
 *                  d'entrée/sortie, qui est gérée par le système d'exploitation, quand elle est terminée,
 *                  le processus est remis automatiquement dans le ready queue.
 * - OS_RUN_PREEMPTED: le processus en cours d'exécution a terminé son quantum/time_slice et doit être
 *                     remis dans le ready queue.
 *
 * @param core le coeur à interroger
 * @param process le nouveau processus
 * @param time_slice le quantum
 */
int os_run_process(process_t *process, uint32_t core, uint64_t time_slice);

/**
 * Cette fonction démarre l'opération d'entrée/sortie pour un processus. Cette fonction est non bloquante,
 * les opérations d'entrée/sortie sont gérées par le système d'exploitation en parallèle, séparément des
 * coeurs.
 *
 * Quand l'opération d'entrée/sortie est terminée, le processus est remis automatiquement dans le ready queue.
 */
void os_start_io(process_t *process);

/**
 * Cette fonction retourne le temps actuel du système.
 *
 * @return le temps actuel du système
 */
uint64_t os_time(void);

/**
 * Cette fonction démarre le système d'exploitation.
 */
int os_start(const char* filename);


#endif
