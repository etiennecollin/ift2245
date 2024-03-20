#include "process.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "os.h"


process_t *create_process(int pid) {
    assert(pid >= 0);
    process_t *process = malloc(sizeof(process_t));
    memset(process, 0, sizeof(process_t));
    process->pid = pid;
    process->burst_length = 0;
    process->io_length = 0;
    process->priority_level = DEFAULT_PRIORITY_LEVEL;
    process->status = -1;
    process->found_burst = 0;
    process->found_io = 0;
    process->already_executed = 0;
    pthread_mutex_init(&process->mutex, NULL);
    return process;
}

void destroy_process(process_t *process) {
    assert(process != NULL);
    free(process);
}