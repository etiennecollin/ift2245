// Copyright 2024 Etienne Collin 20237904 & Justin Villeneuve 20132792

#include "main.h"

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef int error_code;

#define ERROR (-1)
#define HAS_ERROR(code) ((code) < 0)
#define HAS_NO_ERROR(code) ((code) >= 0)

/**
 * Cette fonction compare deux chaînes de caractères.
 * @param p1 la première chaîne
 * @param p2 la deuxième chaîne
 * @return le résultat de la comparaison entre p1 et p2. Un nombre plus petit
 * que 0 dénote que la première chaîne est lexicographiquement inférieure à la
 * deuxième. Une valeur nulle indique que les deux chaînes sont égales tandis
 * qu'une valeur positive indique que la première chaîne est lexicographiquement
 * supérieure à la deuxième.
 */
int strcmp(const char *p1, const char *p2) {
    char c1, c2;
    do {
        c1 = (char) *p1++;
        c2 = (char) *p2++;
        if (c1 == '\0') return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}

/**
 * Ex. 1: Calcul la longueur de la chaîne passée en paramètre selon
 * la spécification de la fonction strlen standard
 * @param s un pointeur vers le premier caractère de la chaîne
 * @return le nombre de caractères dans le code d'erreur, ou une erreur
 * si l'entrée est incorrecte
 */
error_code strlen2(const char *s) {
    // Make sure the pointer is not null
    if (s == NULL) return -1;

    // Initialize the length to 0 and get the first character
    int len = 0;
    int current = *s++;

    // Loop through the string until the null character is reached
    while (current != '\0') {
        len++;
        current = (char) *s++;
    }

    return len;
}

/**
 * Ex.2 :Retourne le nombre de lignes d'un fichier sans changer la position
 * courante dans le fichier.
 * @param fp un pointeur vers le descripteur de fichier
 * @return le nombre de lignes, ou -1 si une erreur s'est produite
 */
error_code no_of_lines(FILE *fp) {
    // Check that the file pointer is not NULL
    if (fp == NULL) return -1;

    // Save the current position in the file
    long pos = ftell(fp);

    // Check that the current position is valid
    if (pos == -1) return -1;

    // Go to the beginning of the file
    rewind(fp);

    // Count the number of lines in the file and keep track of the previous character
    // This is done to check if the last line of the file is empty or not
    int count = 0;
    int current, previous = '\n';
    while ((current = getc(fp)) != EOF) {
        if (current == '\n') {
            count++;
        }
        previous = current;
    }

    // If the last character is not a new line, add one to the count
    if (previous != '\n') {
        count++;
    }

    // Restore the position in the file
    if (fseek(fp, pos, SEEK_SET) == -1) return -1;

    return count;
}

/**
 * Ex.3: Lit une ligne au complet d'un fichier
 * @param fp le pointeur vers la ligne de fichier
 * @param out le pointeur vers la sortie
 * @param max_len la longueur maximale de la ligne à lire
 * @return le nombre de caractère ou ERROR si une erreur est survenue
 */
error_code readline(FILE *fp, char **out, size_t max_len) {
    // Check that the file pointer and out pointer is not NULL
    if (fp == NULL || out == NULL) return -1;

    // Allocate memory for the string
    // We allocate max_len + 1 to account for the null character that terminates the string
    char *addr = malloc(max_len + 1);

    // Check that the malloc was successful
    if (addr == NULL) return -1;

    // Read the line from the file and save it to memory
    int current;
    int i = 0;
    while ((current = getc(fp)) != '\n' && current != EOF && i <= max_len) {
        addr[i++] = current;
    }

    // Add the null character to the end of the string
    addr[i] = '\0';

    // Output the address of the string and return the number of characters read
    *out = addr;
    return i;
}

/**
 * Ex.4: Copie un bloc mémoire vers un autre
 * @param dest la destination de la copie
 * @param src  la source de la copie
 * @param len la longueur (en byte) de la source
 * @return nombre de bytes copiés ou une erreur s'il y a lieu
 */
error_code memcpy2(void *dest, const void *src, size_t len) {
    // Check that the pointers are not NULL
    if (dest == NULL || src == NULL) return -1;

    // Cast the pointers to byte pointers
    byte *dest_addr = (byte *) dest;
    byte *src_addr = (byte *) src;

    // Copy the source to the destination
    for (size_t i = 0; i < len; i++) {
        dest_addr[i] = src_addr[i];
    }

    return (int) len;
}

/**
 * Reads a state from a line, skips the following comma and returns the state
 * @param line the line to read from
 * @param p the position in the line at which the state starts
 * @return the state or NULL if an error occurred
 */
char *read_state(const char *line, size_t *p) {
    // Allocate memory for the current state
    char *current_state = malloc(sizeof(char) * 6);

    // Verify that the malloc was successful
    if (current_state == NULL) {
        return NULL;
    }

    // Read the current state
    size_t i = 0;
    while (line[*p] != ',' && i < 5) {
        current_state[i] = line[(*p)++];
        i++;
    }
    current_state[i] = '\0';

    // Skip the comma
    (*p)++;

    return current_state;
}

/**
 * Reads a char from a line
 * @param line the line to read from
 * @param p the position in the line at which the char starts
 * @return the char or 2 if an error occurred
 */
char parse_movement(const char *line, size_t *p) {
    // Read the symbol to read
    char read = line[(*p)++];

    switch (read) {
        case 'G':
            return -1;
        case 'R':
            return 0;
        case 'D':
            return 1;
        default:
            return 2;
    }
}

/**
 * Ex.5: Analyse une ligne de transition
 * @param line la ligne à lire
 * @param len la longueur de la ligne
 * @return la transition ou NULL en cas d'erreur
 */
transition *parse_line(char *line, size_t len) {
    // Keep the position in the line
    // Initialize the position to 1 to skip the first parenthesis
    size_t p = 1;

    // ====================
    // Current State
    // ====================
    // Read the current state
    char *current_state = read_state(line, &p);
    if (current_state == NULL) return NULL;

    // ====================
    // Read
    // ====================
    // Read the symbol to read
    char read = line[p++];
    p += 4; // Skip the )->(

    // ====================
    // Next State
    // ====================
    // Allocate memory for the current state
    char *next_state = read_state(line, &p);
    if (next_state == NULL) {
        free(current_state);
        return NULL;
    }

    // ====================
    // Write
    // ====================
    // Read the symbol to read and add the null character
    char write = line[p++];
    p++; // Skip the comma

    // ====================
    // Movement
    // ====================
    // Read the movement
    char movement = parse_movement(line, &p);

    // Check that the movement is valid and that we did not reach the end of the line
    if (movement == 2 || p > len) {
        free(current_state);
        free(next_state);
        return NULL;
    }

    // ====================
    // Initialize the transition
    // ====================
    // Allocate memory for a transition
    transition *t = malloc(sizeof(transition));

    // Check that the malloc was successful
    if (t == NULL) {
        free(current_state);
        free(next_state);
        return NULL;
    }

    t->current_state = current_state;
    t->next_state = next_state;
    t->read = read;
    t->write = write;
    t->movement = movement;

    return t;
}

/**
 * Ex.6: Execute la machine de turing dont la description est fournie
 * @param machine_file le fichier de la description
 * @param input la chaîne d'entrée de la machine de turing
 * @return le code d'erreur
 */
error_code execute(char *machine_file, char *input) {
    // Check that the pointers are not NULL
    if (machine_file == NULL || input == NULL) return ERROR;

    // Open the machine file
    FILE *fp = fopen(machine_file, "r");
    if (fp == NULL) return ERROR;

    // Get the number of lines in the machine file
    int num_lines = no_of_lines(fp);
    if (num_lines == ERROR) return ERROR;

    // Read the initial, accept and reject states
    char **initial_state = malloc(sizeof(char *));
    char **accept_state = malloc(sizeof(char *));
    char **reject_state = malloc(sizeof(char *));

    // Check that the mallocs were successful
    if (readline(fp, initial_state, 5) == ERROR
        || readline(fp, accept_state, 5) == ERROR
        || readline(fp, reject_state, 5) == ERROR) {
        free(*initial_state);
        free(*accept_state);
        free(*reject_state);
        free(initial_state);
        free(accept_state);
        free(reject_state);
        return ERROR;
    }

    // Get the length of the input
    int input_length = strlen2(input);
    if (input_length == ERROR) return ERROR;

    // Create the "tape" and set its default length
    int tape_length = input_length * 2;
    if (tape_length < 256) tape_length = 256;

    // Allocate the pointer to the tape
    char **tape = malloc(sizeof(char *));
    if (tape == NULL) return ERROR;

    // Allocate the tape
    *tape = malloc(sizeof(char) * tape_length);
    if (*tape == NULL) {
        free(tape);
        return ERROR;
    }

    // Initialize the tape with spaces
    for (int i = 0; i < tape_length; i++) {
        (*tape)[i] = ' ';
    }

    // Copy the input to the middle of the tape
    int position = tape_length / 2 - input_length / 2;
    memcpy2(&(*tape)[position], input, input_length);

    // Create a transition array
    int num_transitions = num_lines - 3;
    transition **transitions = get_transitions(fp, num_transitions);
    if (transitions == NULL) {
        free(*initial_state);
        free(*accept_state);
        free(*reject_state);
        free(initial_state);
        free(accept_state);
        free(reject_state);
        free(*tape);
        free(tape);
        return ERROR;
    }

    int result = step(tape, tape_length, position, transitions, num_transitions, *initial_state, *accept_state,
                      *reject_state);

    // Close the file
    fclose(fp);
    free(*initial_state);
    free(*accept_state);
    free(*reject_state);
    free(initial_state);
    free(accept_state);
    free(reject_state);
    free(*tape);
    free(tape);
    for (int i = 0; i < num_transitions; i++) {
        free(transitions[i]->current_state);
        free(transitions[i]->next_state);
        free(transitions[i]);
    }
    free(transitions);

    return result;

}

/**
 * Reads the transitions from a file
 * @param fp the file pointer
 * @param num_transitions the number of transitions to read
 * @return the transitions or NULL if an error occurred
 */
transition **get_transitions(FILE *fp, int num_transitions) {
    transition **transitions = malloc(sizeof(transition *) * num_transitions);
    char **current_line = malloc(sizeof(char *));

    // Check that the malloc was successful
    if (transitions == NULL || current_line == NULL) {
        free(transitions);
        free(current_line);
        return NULL;
    }

    // Read the transitions
    for (int i = 0; i < num_transitions; i++) {
        int res = readline(fp, current_line, 1024);
        int len = strlen2(*current_line);

        if (res == ERROR || len == ERROR) {
            free(transitions);
            free(*current_line);
            free(current_line);
            return NULL;
        }

        transition *transition = parse_line(*current_line, len);
        if (transition == NULL) {
            free(transitions);
            free(*current_line);
            free(current_line);
            return NULL;
        }

        transitions[i] = transition;
        free(*current_line);
    }

    free(current_line);
    return transitions;
}

/**
 * Runs the turing machine until it reaches an accept or reject state
 * @param tape the tape of the turing machine
 * @param position the position of the head on the tape
 * @param transitions the list of available transitions
 * @param num_transitions the number of transitions available
 * @param initial_state the initial state of the turing machine
 * @param accept_state the accept state of the turing machine
 * @param reject_state the reject state of the turing machine
 * @return -1 if an error occurred, 0 if the machine rejected the input, 1 if the machine accepted the input
 */
error_code
step(char **tape, int tape_length, int position, transition **transitions, int num_transitions, char *initial_state,
     char *accept_state, char *reject_state) {
    // Get the current state
    char *current_state = initial_state;


    while (strcmp(current_state, accept_state) != 0 && strcmp(current_state, reject_state) != 0) {
        // Get the current symbol
        char current_symbol = (*tape)[position];

        // Find matching transition
        transition *current_transition = NULL;
        for (int i = 0; i < num_transitions; i++) {
            // Get the transition
            transition *transition = transitions[i];

            // Check if the transition is the correct one
            if (strcmp(transition->current_state, current_state) == 0 && transition->read == current_symbol) {
                current_transition = transition;
                break;
            }
        }

        // No transition found
        if (current_transition == NULL) {
            return ERROR;
        }

        // Update the tape
        current_state = current_transition->next_state;
        (*tape)[position] = current_transition->write;
        position += current_transition->movement;

        // Check if the position is valid
        if (position <= 0 || position >= tape_length) {
            if (resize_tape(tape, &tape_length, &position) == ERROR) {
                return ERROR;
            }

        }
    }

    if (strcmp(current_state, accept_state) == 0) {
        return 1;
    } else {
        return 0;
    }
}

error_code resize_tape(char **tape, int *tape_length, int *position) {
    int current_position = *position;
    int current_length = *tape_length;

    // Calculate the new length and allocate memory for the new tape
    int new_length = current_length * 2;
    char *new_tape = malloc(sizeof(char) * new_length);

    // Check that the malloc was successful
    if (new_tape == NULL) {
        return ERROR;
    }

    // Initialize the new tape with spaces
    for (int i = 0; i < new_length; i++) {
        new_tape[i] = ' ';
    }

    // Calculate new position to center the old tape on the new tape
    int tape_offset = current_length / 2;

    // Copy the old tape to the new tape
    if (memcpy2(&new_tape[tape_offset], *tape, current_length) == ERROR) return ERROR;

    // Update the tape, the length and the position
    free(*tape);
    *tape = new_tape;
    *tape_length = new_length;
    *position = tape_offset + current_position;

    return 0;
}

// ATTENTION! TOUT CE QUI EST ENTRE LES BALISES ༽つ۞﹏۞༼つ SERA ENLEVÉ!
// N'AJOUTEZ PAS D'AUTRES ༽つ۞﹏۞༼つ

// ༽つ۞﹏۞༼つ

int main() {
    // ====================
    // Testing ex-1
    // ====================
    printf("Ex-1\n");

    char *str = "";
    int len = strlen2(str);
    printf("├ Test 1 passing? -> %s\n", len == 0 ? "true" : "false");

    str = "\\0";
    len = strlen2(str);
    printf("├ Test 2 passing? -> %s\n", len == 2 ? "true" : "false");

    str = "1";
    len = strlen2(str);
    printf("├ Test 3 passing? -> %s\n", len == 1 ? "true" : "false");

    str = "Hello World!";
    len = strlen2(str);
    printf("├ Test 5 passing? -> %s\n", len == 12 ? "true" : "false");

    printf("└ Done testing Ex-1\n");

    // ====================
    // Testing ex-2
    // ====================
    printf("Ex-2\n");

    // Create an array of the files to test
    const char *files[] = {"../empty", "../five_lines", "../six_lines", "../seven_lines", "../eight_lines"};
    int num_files = sizeof(files) / sizeof(files[0]);

    // Loop over each file
    for (int i = 0; i < num_files; i++) {
        // Open the file
        FILE *fp = fopen(files[i], "r");

        // Check that the file was opened correctly
        if (fp == NULL) {
            printf("├ Failed to open file %s\n", files[i]);
            continue;
        }

        // Get the size of the file
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);

        // Move the file pointer to a random position in the file
        long random_pos = rand() % size;
        fseek(fp, random_pos, SEEK_SET);

        // Save the current position in the file
        long initial_pos = ftell(fp);

        // Call the no_of_lines function and print the result
        error_code result = no_of_lines(fp);
        if (HAS_ERROR(result)) {
            printf("├ An error occurred with file %s\n", files[i]);
        } else {
            printf("├ The file %s has %d lines\n", files[i], result);
        }

        // Check that the position in the file is the same as before
        long final_pos = ftell(fp);
        if (initial_pos != final_pos) {
            printf("├ Error: The position in the file %s changed after calling no_of_lines\n", files[i]);
        }

        // Close the file
        fclose(fp);
    }

    printf("└ Done testing Ex-2\n");

    // ====================
    // Testing ex-3
    // ====================
    printf("Ex-3\n");

    // Allocate memory for the string
    char **read = malloc(sizeof(char *));

    // Open the file
    FILE *fp = fopen("../five_lines", "r");

    // Check that the file was opened correctly
    if (fp == NULL) {
        printf("├ Failed to open file %s\n", "../five_lines");
    }

    // Read the first line of the file
    len = readline(fp, read, 1024);

    // Check that the line was read correctly
    if (len == -1) { printf("├ An error occurred while reading line one in %s\n", "../five_lines"); }
    char *line = *read;
    printf("├ Test 1 passing? -> %s\n", len == 8 && strcmp(line, "line one") == 0 ? "true" : "false");
    free(line);

    len = readline(fp, read, 1024);
    if (len == -1) { printf("├ An error occurred while reading line two in %s\n", "../five_lines"); }
    line = *read;
    printf("├ Test 2 passing? -> %s\n", len == 8 && strcmp(line, "line two") == 0 ? "true" : "false");
    free(line);

    len = readline(fp, read, 1024);
    if (len == -1) { printf("├ An error occurred while reading line three in %s\n", "../five_lines"); }
    line = *read;
    printf("├ Test 3 passing? -> %s\n", len == 10 && strcmp(line, "line three") == 0 ? "true" : "false");
    free(line);

    len = readline(fp, read, 1024);
    if (len == -1) { printf("├ An error occurred while reading line four in %s\n", "../five_lines"); }
    line = *read;
    printf("├ Test 4 passing? -> %s\n", len == 9 && strcmp(line, "line four") == 0 ? "true" : "false");
    free(line);

    len = readline(fp, read, 1024);
    if (len == -1) { printf("├ An error occurred while reading line five in %s\n", "../five_lines"); }
    line = *read;
    printf("├ Test 5 passing? -> %s\n", len == 9 && strcmp(line, "line five") == 0 ? "true" : "false");
    free(line);

    len = readline(fp, read, 1024);
    if (len == -1) { printf("├ An error occurred while reading line six in %s\n", "../five_lines"); }
    line = *read;
    printf("├ Test 6 passing? -> %s\n", len == 0 && strcmp(line, "") == 0 ? "true" : "false");
    free(line);

    free(read);
    fclose(fp);

    printf("└ Done testing Ex-3\n");

    // ====================
    // Testing ex-4
    // ====================
    printf("Ex-4\n");

    byte *a = calloc(100, sizeof(byte));
    byte *b = calloc(100, sizeof(byte));
    for (int i = 0; i < 10; i++) b[i] = i + 1;

    int nb_bytes = memcpy2(a, b, 10);
    printf("├ Test 0 passing? -> %s\n", nb_bytes == 10 ? "true" : "false");

    int passing = 1;
    for (int i = 0; i < 10; i++) {
        passing = a[i] == b[i] && passing;
    }
    printf("├ Test 1 passing? -> %s\n", passing == 1 ? "true" : "false");

    for (int i = 0; i < 100; ++i) {
        b[i] = 0;
        a[i] = 0;
    }
    for (int i = 0; i < 100; i++) b[i] = i + 1;

    nb_bytes = memcpy2(a, b, 100);
    printf("├ Test 2 passing? -> %s\n", nb_bytes == 100 ? "true" : "false");

    passing = 1;
    for (int i = 0; i < 100; i++) {
        passing = a[i] == b[i] && passing;
    }
    printf("├ Test 3 passing? -> %s\n", passing == 1 ? "true" : "false");

    free(a);
    free(b);

    a = malloc(sizeof(byte) * 0);
    b = malloc(sizeof(byte) * 0);

    nb_bytes = memcpy2(a, b, 0);
    printf("├ Test 4 passing? -> %s\n", nb_bytes == 0 ? "true" : "false");

    free(a);
    free(b);

    printf("└ Done testing Ex-4\n");

    // ====================
    // Testing ex-5
    // ====================
    printf("Ex-5\n");
    line = "(q0,0)->(qR,0,D)\n";
    transition *t = parse_line(line, strlen2(line));
    int result = strcmp(t->current_state, "q0") == 0;
    result &= strcmp(t->next_state, "qR") == 0;
    result &= t->read == '0';
    result &= t->write == '0';
    result &= t->movement == 1;
    printf("├ Test 1 passing? -> %s\n", result == 1 ? "true" : "false");
    free(t->next_state);
    free(t->current_state);
    free(t);

    line = "(q0,1)->(qA,1,R)";
    t = parse_line(line, strlen2(line));
    result = strcmp(t->current_state, "q0") == 0;
    result &= strcmp(t->next_state, "qA") == 0;
    result &= t->read == '1';
    result &= t->write == '1';
    result &= t->movement == (char) 0;
    printf("├ Test 2 passing? -> %s\n", result == 1 ? "true" : "false");
    free(t->next_state);
    free(t->current_state);
    free(t);

    line = "(q0123,1)->(q,1,R)";
    t = parse_line(line, strlen2(line));
    result = strcmp(t->current_state, "q0123") == 0;
    result &= strcmp(t->next_state, "q") == 0;
    result &= t->read == '1';
    result &= t->write == '1';
    result &= t->movement == (char) 0;
    printf("├ Test 3 passing? -> %s\n", result == 1 ? "true" : "false");
    free(t->next_state);
    free(t->current_state);
    free(t);

    line = "(q0123,1)->(q,1,G)";
    t = parse_line(line, strlen2(line));
    result = strcmp(t->current_state, "q0123") == 0;
    result &= strcmp(t->next_state, "q") == 0;
    result &= t->read == '1';
    result &= t->write == '1';
    result &= t->movement == (char) -1;
    printf("├ Test 4 passing? -> %s\n", result == 1 ? "true" : "false");
    free(t->next_state);
    free(t->current_state);
    free(t);

    line = "(q0,1)->(qA,1,R)";
    t = parse_line(line, strlen2(line));
    printf("├ Test 5 passing? -> %s\n", t != NULL ? "true" : "false");
    if (t != NULL) {
        free(t->next_state);
        free(t->current_state);
    }
    free(t);

    printf("└ Done testing Ex-5\n");

    // ====================
    // Testing ex-6
    // ====================
    printf("Ex-6\n");

    printf("├ Test 1 passing? -> %s\n", execute("../this_file_dne", "101010") == -1 ? "true" : "false");
    printf("├ Test 2 passing? -> %s\n", execute("../youre_gonna_go_far_kid", "") == 1 ? "true" : "false");
    printf("├ Test 3 passing? -> %s\n", execute("../has_five_ones", "0000") == 0 ? "true" : "false");
    printf("├ Test 4 passing? -> %s\n", execute("../has_five_ones", "101010101") == 1 ? "true" : "false");
    printf("├ Test 5 passing? -> %s\n", execute("../has_five_ones", "111111111") == 1 ? "true" : "false");
    printf("├ Test 6 passing? -> %s\n",
           execute("../youre_gonna_go_far_kid", "STARING AT THE SUN") == -1 ? "true" : "false");
    printf("└ Done testing Ex-6\n");

    return 0;
}

// ༽つ۞﹏۞༼つ
