
#ifndef TP1_PARSER_H
#define TP1_PARSER_H

#include "tokenizer.h"

enum op {
    OP_TERMINATOR = 0,
    OP_SEPARATOR,
    OP_AND,
    OP_OR,
    OP_PIPE,
};

struct command {
    struct command *next; // Commande suivante
    char **args; // Tableau de chaînes de caractères, le dernier élément est NULL
    enum op op; // Opérateur
};

/**
 * Cette fonction prend une liste de tokens et retourne une liste de commandes.
 *
 * @param tokens list chainée de tokens
 * @return list chainée de commandes
 */
struct command *cmd_parse(struct token *tokens);

/**
 * Cette fonction libère la mémoire allouée pour une liste de commandes.
 *
 * @param command list chainée de commandes
 */
void cmd_free(struct command *command);

/**
 * Cette fonction affiche une liste de commandes.
 * Utilisé pour le débogage.
 *
 * @param commands list chainée de commandes
 */
void cmd_debug_print(const struct command *commands);


#endif
