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
 * @return the execution status of the command, i.e., execution_failed or execution_success
 */
int execute_command(const struct command *cmd, enum op previous_op, int previous_result, int pipe_fd[2]) {
    // Check for "OR" and "AND" operators
    if (previous_op == OP_AND && previous_result == EXECUTION_FAILED) {
        return EXECUTION_SUCCESS;
    } else if (previous_op == OP_OR && previous_result == EXECUTION_SUCCESS) {
        return EXECUTION_SUCCESS;
    }

    // Check for exit command
    if (strcmp(cmd->args[0], "exit") == 0) return EXECUTION_REQUEST_EXIT;

    // Fork and execute the command
    int status = 0;
    pid_t pid = fork();
    if (pid < 0) { // error occurred
        perror("Fork failed");
        return EXECUTION_FAILED;
    }

    // Run the child process
    if (pid == 0) {
        // Set the input and output
        if (previous_op == OP_PIPE) {
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[0]);
        }
        if (cmd->op == OP_PIPE) {
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);
        }

        // Execute the command
        int return_value = execvp(cmd->args[0], cmd->args);

        // If execvp returns, it must have failed
        if (return_value != 0) {
            printf("%s: command not found\n", cmd->args[0]);
            exit(EXECUTION_FAILED);
        }
    } else {
        // Wait for the child process to finish
        waitpid(pid, &status, 0);
    }

    // Check exit status
    if (status == 0) {
        return EXECUTION_SUCCESS;
    } else {
        return EXECUTION_FAILED;
    }
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

    // Initialize the previous operator
    enum op previous_op = (enum op) NULL;
    int previous_result = EXECUTION_SUCCESS;

    // Create a pipe
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return EXECUTION_FAILED;
    }

    // Execute the commands
    struct command *current = cmd;
    while (current != NULL) {
        previous_result = execute_command(current, previous_op, previous_result, pipe_fd);
        close(pipe_fd[1]);

        // Check for exit command
        if (previous_result == EXECUTION_REQUEST_EXIT) return EXECUTION_REQUEST_EXIT;

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
