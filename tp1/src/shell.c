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

int is_skipped_op(enum op op) {
    return !(op == OP_AND || op == OP_OR || op == OP_SEPARATOR);
}

/**
 * Creates a new process
 * @return the execution status of the command, i.e., execution_failed or execution_success
 */
int execute_command(const struct command *cmd, enum op previous_op, int previous_result, int *is_skipping,
                    int *previous_pipe_output) {
    // If the previous command failed and the operator is "AND", skip the current command
    // If the previous command succeeded and the operator is "OR", skip the current command
    // If we are skipping and the current operator is "skipped operator", skip the current command
    // Otherwise, reset the skipping flag
    if ((previous_op == OP_AND && previous_result == EXECUTION_FAILED) ||
        (previous_op == OP_OR && previous_result == EXECUTION_SUCCESS)) {
        *is_skipping = 1;
        return previous_result;
    } else if (*is_skipping == 1 && is_skipped_op(previous_op)) {
        return previous_result;
    } else {
        *is_skipping = 0;
    }

    // Check for exit command
    if (strcmp(cmd->args[0], "exit") == 0) return EXECUTION_REQUEST_EXIT;

    // Create a pipe
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "Error creating a pipe\n");
        exit(EXECUTION_FAILED);
    }

    // Fork and execute the command
    int status = 0;
    pid_t pid = fork();
    if (pid < 0) { // error occurred
        fprintf(stderr, "Fork failed\n");
        return EXECUTION_FAILED;
    }

    // Run the child process
    if (pid == 0) {
        // Set STDIN to the previous pipe output
        if (previous_op == OP_PIPE) {
            dup2(*previous_pipe_output, STDIN_FILENO);
        }

        // Set STDOUT to the write end of the pipe
        if (cmd->op == OP_PIPE) {
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);
        }

        // Execute the command
        int return_value = execvp(cmd->args[0], cmd->args);

        // If execvp returns, it must have failed
        if (return_value != 0) {
            fprintf(stderr, "%s: command not found\n", cmd->args[0]);
            exit(EXECUTION_FAILED);
        }
    } else {
        // Wait for the child process to finish
        waitpid(pid, &status, 0);

        // Close the write end of the pipe and update the previous pipe output
        close(pipe_fd[1]);
        *previous_pipe_output = pipe_fd[0];
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

    // Initialize variables
    enum op previous_op = (enum op) NULL;
    int previous_result = EXECUTION_SUCCESS;
    int is_skipping = 0;
    int previous_pipe_output = STDIN_FILENO;

    // Execute the commands
    struct command *current_command = cmd;
    while (current_command != NULL) {
        // Execute the command
        previous_result = execute_command(current_command, previous_op, previous_result, &is_skipping,
                                          &previous_pipe_output);

        // Check for exit command
        if (previous_result == EXECUTION_REQUEST_EXIT) return EXECUTION_REQUEST_EXIT;

        previous_op = current_command->op;
        current_command = current_command->next;
    }

    return EXECUTION_SUCCESS;
}

int main(void) {
    while (1) {
        struct token *tokens = tok_next_line();

        if (!tokens) continue;

//        tok_debug_print(tokens);

        struct command *commands = cmd_parse(tokens);

        if (!commands) {
            tok_free(tokens);
            continue;
        }

//        cmd_debug_print(commands);

        int status = sh_run(commands);
        cmd_free(commands);
        tok_free(tokens);

        if (status == EXECUTION_REQUEST_EXIT) {
            exit(0);
        }
    }
}
