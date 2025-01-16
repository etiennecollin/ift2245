#include "os.h"

#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ready_queue.h"
#include "worker.h"

////
//// Scheduler internal data structures
////

typedef struct burst burst_t;

struct burst {
    uint64_t cpu_time;
    uint64_t io_time;
    uint64_t remaining;
    uint64_t arrival;
    uint64_t completion;
    uint64_t first_run;
};

typedef struct process_info process_info_t;

struct process_info {
    process_t *process;

    burst_t *bursts;

    uint64_t burst_cpu_index;
    uint64_t burst_io_index;
    uint64_t burst_count;

    int is_new;
    int is_waiting;
    uint64_t is_ready_at;
};

typedef struct cpu_ctx cpu_ctx_t;

struct cpu_ctx {
    worker_t *worker;
    process_t *last;
    uint64_t total_ctx_switches;
};

////
//// Scheduler state
////

static process_info_t *process_infos;
static uint64_t process_count;
static volatile uint64_t process_done_count;

static ready_queue_t ready_queue;

static cpu_ctx_t cpu_ctxs[OS_CORES];

static pthread_mutex_t mutex;

////
//// Scheduler API
////

int os_switch_process(cpu_ctx_t *cpu, process_t *process) {
    if (cpu->last == process) return 0;

    // Perform the context switch
    // This cost is here to simulate the time it takes to perform a context switch
    usleep(OS_CONTEXT_SWITCH_DURATION * 1000);

    cpu->last = process;
    ++cpu->total_ctx_switches;

    return 1;
}

int os_run_user_mode(process_t *process, uint64_t time_slice) {
    pthread_mutex_lock(&mutex);

    process_info_t *info = &process_infos[process->pid];

    uint64_t current = info->burst_cpu_index;
    uint64_t length = info->burst_count;

    assert(current < length); // Process already done

    burst_t burst = info->bursts[current];

    pthread_mutex_unlock(&mutex);

    int preempted = time_slice > 0 && time_slice < burst.remaining;
    int done = !preempted && current + 1 >= length;

    uint64_t time = preempted ? time_slice : burst.remaining;
    uint64_t t1 = os_time();
    usleep(time * 1000);
    uint64_t t2 = os_time();

    pthread_mutex_lock(&mutex);

    if (burst.cpu_time == burst.remaining) {
        info->bursts[current].first_run = t1;
    }

    if (preempted) {
        info->bursts[current].remaining -= time;
    } else {
        info->bursts[current].remaining = 0;
        info->bursts[current].completion = t2;
        info->burst_cpu_index++;

        if (done) {
            process_done_count++;
        }
    }

    pthread_mutex_unlock(&mutex);

    return preempted ? OS_RUN_PREEMPTED : done ? OS_RUN_DONE : OS_RUN_BLOCKED;
}

int os_run_process(process_t *process, uint32_t core, uint64_t time_slice) {
    assert(core < OS_CORES); // Core number is not in range
    assert(process != NULL); // Process is NULL

    cpu_ctx_t *ctx = &cpu_ctxs[core];

    // Check if the process is different from the last one

    os_switch_process(ctx, process);

    // Run the process

    int status = os_run_user_mode(process, time_slice);

    if (status == OS_RUN_DONE) {
        printf("Process %d completed (%lu / %lu)\n", process->pid, process_done_count, process_count);
    }

    return status;
}

void os_start_io(process_t *process) {
    uint64_t now = os_time();

    pthread_mutex_lock(&mutex);

    process_info_t *info = &process_infos[process->pid];

    assert(info->burst_io_index + 1 == info->burst_cpu_index); // IO burst is not next

    info->is_ready_at = now + info->bursts[info->burst_io_index].io_time;
    info->is_waiting = 1;

    info->burst_io_index++;

    pthread_mutex_unlock(&mutex);

    printf("Process %d IO\n", process->pid);
}

uint64_t os_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

////
//// Scheduler Main
////

void os_parse_file(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Error: could not open file\n");
        exit(1);
    }

    fscanf(file, "%lu", &process_count);
    process_done_count = 0;

    uint64_t now = os_time();

    process_infos = malloc(sizeof(process_info_t) * process_count);

    for (int i = 0; i < process_count; i++) {
        process_info_t *info = &process_infos[i];

        uint64_t pid;
        uint64_t arrival;
        uint64_t burst_count;
        uint64_t burst_time;
        uint64_t io_time;

        fscanf(file, "%lu; %lu; %lu; %lu; %lu", &pid, &arrival, &burst_count, &burst_time, &io_time);

        info->burst_cpu_index = 0;
        info->burst_io_index = 0;
        info->burst_count = burst_count;
        info->is_new = 1;
        info->is_waiting = 1;
        info->is_ready_at = now + arrival;

        info->bursts = malloc(sizeof(burst_t) * burst_count);

        for (int j = 0; j < burst_count; j++) {
            info->bursts[j].cpu_time = burst_time;
            info->bursts[j].io_time = io_time;
            info->bursts[j].remaining = burst_time;
            info->bursts[j].arrival = 0;
            info->bursts[j].completion = 0;
            info->bursts[j].first_run = 0;
        }
    }

    fclose(file);
}

void os_loop() {
    for (;;) {
        pthread_mutex_lock(&mutex);

        int done = process_done_count >= process_count;

        pthread_mutex_unlock(&mutex);

        if (done) break;

        for (int i = 0; i < process_count; i++) {
            uint64_t now = os_time();

            pthread_mutex_lock(&mutex);

            process_info_t *info = &process_infos[i];

            int move_to_ready = (info->is_waiting && info->is_ready_at <= now) ||
                                (info->is_new && info->is_ready_at <= now);

            int create_new_process = info->is_new;

            if (move_to_ready) {
                if (create_new_process) {
                    assert(info->burst_cpu_index == 0);
                }

                info->bursts[info->burst_cpu_index].arrival = now;
                info->is_waiting = 0;
                info->is_new = 0;
            }

            pthread_mutex_unlock(&mutex);

            if (move_to_ready) {
                if (create_new_process) {
                    info->process = create_process(i);
                }

                process_t *process = info->process;

                ready_queue_push(&ready_queue, process);

                if (create_new_process) {
                    printf("Process %d created \n", i);
                } else {
                    printf("Process %d ready \n", i);
                }
            }
        }

        usleep(500);
    }
}

void os_print_report(void) {
    printf("================================\n");
    printf("REPORT\n");
    printf("================================\n");

    uint64_t total_burst_time = 0;
    uint64_t total_turnaround_time = 0;
    uint64_t total_wait_time = 0;
    uint64_t total_response_time = 0;
    uint64_t max_response_time = 0;
    uint64_t total_bursts = 0;

    uint64_t total_ctx_switches = 0;

    for (int i = 0; i < OS_CORES; i++) {
        total_ctx_switches += cpu_ctxs[i].total_ctx_switches;
    }

    for (int i = 0; i < process_count; i++) {
        process_info_t *info = &process_infos[i];

        printf("Process %d\n", i);

        uint64_t total_process_burst_time = 0;
        uint64_t total_process_turnaround_time = 0;
        uint64_t total_process_wait_time = 0;
        uint64_t total_process_response_time = 0;

        uint64_t bursts = info->burst_count;

        for (int j = 0; j < bursts; j++) {
            burst_t *burst = &info->bursts[j];

            uint64_t burst_time = burst->cpu_time;
            uint64_t turnaround_time = burst->completion - burst->arrival;
            uint64_t wait_time = turnaround_time - burst->cpu_time;
            uint64_t response_time = burst->first_run - burst->arrival;

            total_process_burst_time += burst_time;
            total_process_turnaround_time += turnaround_time;
            total_process_wait_time += wait_time;
            total_process_response_time += response_time;

            if (response_time > max_response_time) {
                max_response_time = response_time;
            }
        }

        uint64_t avg_process_burst_time = total_process_burst_time / bursts;
        uint64_t avg_process_turnaround_time = total_process_turnaround_time / bursts;
        uint64_t avg_process_wait_time = total_process_wait_time / bursts;
        uint64_t avg_process_response_time = total_process_response_time / bursts;

        total_burst_time += total_process_burst_time;
        total_turnaround_time += total_process_turnaround_time;
        total_wait_time += total_process_wait_time;
        total_response_time += total_process_response_time;
        total_bursts += bursts;

        printf("\tAverage burst time: %lu\n", avg_process_burst_time);
        printf("\tAverage turnaround time: %lu\n", avg_process_turnaround_time);
        printf("\tAverage wait time: %lu\n", avg_process_wait_time);
        printf("\tAverage response time: %lu\n", avg_process_response_time);
    }

    printf("================================\n");
    printf("SUMMARY: \n");
    printf("================================\n");

    uint64_t avg_burst_time = total_burst_time / total_bursts;
    uint64_t avg_turnaround_time = total_turnaround_time / total_bursts;
    uint64_t avg_wait_time = total_wait_time / total_bursts;
    uint64_t avg_response_time = total_response_time / total_bursts;

    double score = ((double)avg_wait_time + (double)avg_response_time * 2) / (double)avg_burst_time;

    printf("Average burst time: %lu\n", avg_burst_time);
    printf("Average turnaround time: %lu\n", avg_turnaround_time);
    printf("Average wait time: %lu\n", avg_wait_time);
    printf("Average response time: %lu\n", avg_response_time);
    printf("Max response time: %lu\n", max_response_time);
    printf("Total context switches: %lu\n", total_ctx_switches);

    printf("\nScore: %.2f\n", score);
}


int os_start(const char *filename) {
    printf("================================\n");
    printf("Trace\n");
    printf("Running scheduler with file: %s\n", filename);
    printf("================================\n");

    // Parse the file

    os_parse_file(filename);

    // Initialize the mutex

    pthread_mutex_init(&mutex, NULL);

    // Initialize the ready queue

    ready_queue_init(&ready_queue);

    // Initialize Cpu contexts

    for (int i = 0; i < OS_CORES; i++) {
        cpu_ctxs[i].last = NULL;
        cpu_ctxs[i].total_ctx_switches = 0;
        cpu_ctxs[i].worker = worker_create(i, &ready_queue);
    }

    // Handle the processes until they are all done

    os_loop();

    // Send poison pills to the ready queue

    for (int i = 0; i < OS_CORES; i++) {
        ready_queue_push(&ready_queue, NULL);
    }

    // Wait for all workers to finish

    for (int i = 0; i < OS_CORES; i++) {
        worker_join(cpu_ctxs[i].worker);
    }

    // Print the report

    os_print_report();

    // Clean up

    for (int i = 0; i < OS_CORES; i++) {
        worker_destroy(cpu_ctxs[i].worker);
    }

    ready_queue_destroy(&ready_queue);

    pthread_mutex_destroy(&mutex);

    for (int i = 0; i < process_count; i++) {
        destroy_process(process_infos[i].process);
    }

    for (int i = 0; i < process_count; i++) {
        free(process_infos[i].bursts);
    }

    free(process_infos);

    return 0;
}



