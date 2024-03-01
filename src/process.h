#ifndef TP2_PROCESS_H
#define TP2_PROCESS_H

#include <stdint.h>

typedef struct process process_t;

struct process
{
    int pid; // Don't change this!

    // TODO: add more fields here if needed
};

/**
 * Cette fonction crée un nouveau processus.
 *
 * @param pid le PID du processus
 *
 * @return le processus créé
 */
process_t* create_process(int pid);

/**
 * Cette fonction détruit un processus.
 *
 * @param process le processus à détruire
 */
void destroy_process(process_t* process);

#endif
