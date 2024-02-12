#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/wait.h>

#include "tokenizer.h"
#include "parser.h"

#define EXECUTION_FAILED (-1)
#define EXECUTION_SUCCESS 1
#define EXECUTION_REQUEST_EXIT 0

/**
 * Cette fonction prend une liste de commandes et l'exécute.
 *
 * @param cmd list chainée de commandes
 *
 * @return le code de retour de la dernière commande exécutée.
 */
int sh_run(const struct command *cmd) {
    if (!cmd || cmd->args[0] == NULL) return EXECUTION_FAILED; // Empty command
    if (strcmp(cmd->args[0], "exit") == 0) return EXECUTION_REQUEST_EXIT; // Exit command

    // TODO

    return EXECUTION_REQUEST_EXIT;
}

int main(void) {
    while (1) {
        struct token *tokens = tok_next_line();

        if (!tokens) return EXECUTION_FAILED; // Tokenizer error

//        tok_debug_print(tokens);

        struct command *commands = cmd_parse(tokens);

        if (!commands) {
            tok_free(tokens);
            return EXECUTION_FAILED; // Parser error
        }

//        cmd_debug_print(commands);

        int status = sh_run(commands);

        if (status == EXECUTION_REQUEST_EXIT) {
            exit(0);
        }

        cmd_free(commands);
        tok_free(tokens);
    }
}
