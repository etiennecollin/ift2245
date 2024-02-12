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
 * Creates a new process
 * @return the status of the child
 */
int execute_command(const struct command *cmd, enum op previous_op) {
    pid_t pid = fork();
    int status = 0;
    int pipefd[2];
    pipe(pipefd);

    if (pid < 0) { // error occurred
        perror("Fork failed");
        return EXECUTION_FAILED;
    } else if (pid == 0) { // child process

        // Set the input and output
        if (previous_op == OP_PIPE) {
            dup2(pipefd[0], STDIN_FILENO);
        }
        if (cmd->op == OP_PIPE) {
            dup2(pipefd[1], STDOUT_FILENO);
        }

        int result = execvp(cmd->args[0], cmd->args); // returns -1 if the command failed
        if (result == EXECUTION_FAILED) {
            printf("%s: command not found\n", cmd->args[0]);
        }
    } else { // parent process
        waitpid(pid, &status, 0);
    }

    return status;
}

/**
 * Cette fonction prend une liste de commandes et l'exécute.
 *
 * @param cmd list chainée de commandes
 *
 * @return le code de retour de la dernière commande exécutée.
 */
int sh_run(struct command *cmd) {
    if (!cmd || cmd->args[0] == NULL) return EXECUTION_FAILED; // Empty command
    if (strcmp(cmd->args[0], "exit") == 0) return EXECUTION_REQUEST_EXIT; // Exit command

    // Initialize the previous operator
    enum op previous_op = (enum op) NULL;

    // Execute the commands
    struct command *current = cmd;
    while (current != NULL) {
        execute_command(current, previous_op);
        previous_op = current->op;
        current = current->next;
    }

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
