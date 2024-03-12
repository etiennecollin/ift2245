#ifndef TP2_PROCESS_H
#define TP2_PROCESS_H

#define BASE_PRIORITY_LEVEL 0
#define MIN_PRIORITY_LEVEL 1
#define MAX_PRIORITY_LEVEL 2
#define NUM_PRIORITY_LEVELS 3

#include <stdint.h>

typedef struct process process_t;

struct process
{
    int pid; // Don't change this!

    // TODO: add more fields here if needed
    uint64_t burst_length;
    int priority_level;
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
