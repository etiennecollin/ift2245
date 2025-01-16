//
// Created by charlie on 1/9/21.
//
#include <stdlib.h>
#include <stdio.h>

#ifndef TP0_MAIN_H
#define TP0_MAIN_H

#endif //TP0_MAIN_H
typedef unsigned char byte;
typedef int error_code;

/**
 * Structure qui dénote une transition de la machine de Turing
 */
typedef struct {
    char *current_state;
    char *next_state;
    char movement;
    char read;
    char write;
} transition;

transition **get_transitions(FILE *fp, int num_transitions);

error_code
step(char **tape, int tape_length, int position, transition **transitions, int num_transitions, char *initial_state,
     char *accept_state, char *reject_state);

error_code resize_tape(char **tape, int *tape_length, int *position);

error_code strlen2(const char *s);

error_code no_of_lines(FILE *fp);

error_code readline(FILE *fp, char **out, size_t max_len);

error_code memcpy2(void *dest, const void *src, size_t len);

transition *parse_line(char *line, size_t len);

error_code execute(char *machine_file, char *input);