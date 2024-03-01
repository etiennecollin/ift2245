#include "process.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "os.h"

process_t *create_process(int pid) {
    assert(pid >= 0);
    process_t *process = malloc(sizeof(process_t));
    memset(process, 0, sizeof(process_t));
    process->pid = pid;
    return process;
}

void destroy_process(process_t *process) {
    assert(process != NULL);
    free(process);
}