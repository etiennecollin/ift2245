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
error_code strlen2(const char *s) { return ERROR; }

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

    // Count the number of lines in the file
//    int count = 0;
//    char current, previous = '\n';
//    while ((current = getc(fp)) != EOF) {
//        if (current == '\n') {
//            count++;
//        }
//        previous = current;
//    }
//
//    // If the last character is not a new line, add one to the count
//    if (previous != '\n') {
//        count++;
//    }

    int count = 0;
    char current;
    while ((current = getc(fp)) != EOF) {
        if (current == '\n') {
            count++;
        }
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
error_code readline(FILE *fp, char **out, size_t max_len) { return ERROR; }

/**
 * Ex.4: Copie un bloc mémoire vers un autre
 * @param dest la destination de la copie
 * @param src  la source de la copie
 * @param len la longueur (en byte) de la source
 * @return nombre de bytes copiés ou une erreur s'il y a lieu
 */
error_code memcpy2(void *dest, const void *src, size_t len) { return ERROR; }

/**
 * Ex.5: Analyse une ligne de transition
 * @param line la ligne à lire
 * @param len la longueur de la ligne
 * @return la transition ou NULL en cas d'erreur
 */
transition *parse_line(char *line, size_t len) { return NULL; }

/**
 * Ex.6: Execute la machine de turing dont la description est fournie
 * @param machine_file le fichier de la description
 * @param input la chaîne d'entrée de la machine de turing
 * @return le code d'erreur
 */
error_code execute(char *machine_file, char *input) { return ERROR; }

// ATTENTION! TOUT CE QUI EST ENTRE LES BALISES ༽つ۞﹏۞༼つ SERA ENLEVÉ!
// N'AJOUTEZ PAS D'AUTRES ༽つ۞﹏۞༼つ

// ༽つ۞﹏۞༼つ

int main() {

    // Create an array of the files to test
    const char *files[] = {"../empty", "../five_lines", "../six_lines", "../seven_lines", "../eight_lines"};
    int num_files = sizeof(files) / sizeof(files[0]);

    // Loop over each file
    for (int i = 0; i < num_files; i++) {
        // Open the file
        FILE *fp = fopen(files[i], "r");

        // Check that the file was opened correctly
        if (fp == NULL) {
            printf("Failed to open file %s\n", files[i]);
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
            printf("An error occurred with file %s\n", files[i]);
        } else {
            printf("The file %s has %d lines\n", files[i], result);
        }

        // Check that the position in the file is the same as before
        long final_pos = ftell(fp);
        if (initial_pos != final_pos) {
            printf("Error: The position in the file %s changed after calling no_of_lines\n", files[i]);
        }

        // Close the file
        fclose(fp);
    }
    return 0;
}

// ༽つ۞﹏۞༼つ
