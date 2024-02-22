#include "parser.h"
#include "tokenizer.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/**
 * Determines whether a token is an argument.
 * @param category the category of the token
 * @return true if the token parsed is an argument for a command. False otherwise.
 */
int is_arg(enum token_category category) {
    if (category == TOK_SYMBOL || category == TOK_STRING_LITERAL)
        return 1;
    else return 0;
}

/**
 * Determines whether a token is a separator that is invalid as the first token of a command.
 * @param category the category of the token
 * @return true if the token parsed is an argument for a command. False otherwise.
 */
int is_invalid_first_sep(enum token_category category) {
    if (category == TOK_PIPE || category == TOK_LOGICAL_AND || category == TOK_LOGICAL_OR)
        return 1;
    else return 0;
}

/**
 *
 * Counts the number of arguments in a command to determine how much memory to allocate.
 * @param tokens points to a token that is the first argument of a command
 * @return the number of arguments in a command
 */
int count_arguments(struct token *tokens) {
    struct token *current = tokens;

    int counter = 0;
    while (current != NULL && is_arg(current->category)) {
        counter++;
        current = current->next;
    }

    return counter;
}

/**
 * Finds the operator corresponding to the token category.
 * @return the op corresponding to the token category or -1 if error
 */
enum op find_op(enum token_category category) {
    switch (category) {
        case TOK_SEMICOLON:
            return OP_SEPARATOR;
        case TOK_PIPE:
            return OP_PIPE;
        case TOK_LOGICAL_AND:
            return OP_AND;
        case TOK_LOGICAL_OR:
            return OP_OR;
        case TOK_NEWLINE:
            return OP_TERMINATOR;
        default: // Handles tok invalid, tok symbol, tok string literal
            fprintf(stderr, "Invalid operator token category\n");
            return EXIT_FAILURE;
    }
}

/**
 * Parses a linked list of tokens into a linked list of commands.
 * @param tokens points to the first token in the linked list of tokens
 * @return A linked list of commands
 */
struct command *cmd_parse(struct token *tokens) {
    struct command sentinel = {NULL, NULL, OP_TERMINATOR}; // Head of linked list
    struct command *current_command_in_list = &sentinel;

    if (is_invalid_first_sep(tokens->category)) {
        fprintf(stderr, "Parsing error: first token is not valid\n");
        return NULL;
    }

    if (tokens->category == TOK_NEWLINE) {
        return NULL;
    }

    while (tokens != NULL) {
        if (tokens->category == TOK_INVALID) {
            cmd_free(sentinel.next);
            return NULL;
        }

        // Allocate new_command
        struct command *new_command = malloc(sizeof(struct command));

        // Check that memory allocation was successful
        if (new_command == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            return NULL;
        }

        // Initialize new_command
        new_command->next = NULL;
        new_command->args = NULL;
        new_command->op = OP_TERMINATOR;

        // Get number of tokens
        int arguments_count = count_arguments(tokens);

        // Check if there are no arguments
        if (arguments_count == 0) {
            if (is_invalid_first_sep(tokens->category)) {
                fprintf(stderr, "Parsing error: no arguments\n");
                cmd_free(sentinel.next);
                return NULL;
            } else {
                free(new_command);
                tokens = tokens->next;
                continue;
            }
        }

        // Allocate memory for command args
        new_command->args = malloc(sizeof(new_command->args) * (arguments_count + 1));

        // Check that memory allocation was successful
        if (new_command->args == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            cmd_free(sentinel.next);
            return NULL;
        }

        // Store arguments
        for (int i = 0; i < arguments_count; i++) {
            new_command->args[i] = tokens->value;
            tokens = tokens->next;
        }
        new_command->args[arguments_count] = NULL; // Last element of args array is NULL

        // Get operator
        new_command->op = find_op(tokens->category);
        tokens = tokens->next;

        // Add the new command to the linked list of commands
        current_command_in_list->next = new_command;
        current_command_in_list = new_command;
    }

    return sentinel.next;
}


/**
 * Iterates on the linked list of commands.
 * For each command, deallocates the memory for the arguments. Then, deallocates the memory for the command.
 * @param command the sentinel of the linked list of commands
 */
void cmd_free(struct command *command) {
    struct command *current = command;

    while (current != NULL) {
        // Deallocate memory of args array
        // The args themselves are deallocated by the tokenizer
        free(current->args);

        // Deallocate command
        struct command *temp = current; // Keep reference of old command to deallocate it
        current = current->next;
        free(temp);
    }
}

void cmd_debug_print(const struct command *commands) {
    for (const struct command *cmd = commands; cmd; cmd = cmd->next) {
        for (int i = 0; cmd->args[i]; i++) {
            printf("%s ", cmd->args[i]);
        }

        switch (cmd->op) {
            case OP_TERMINATOR:
                printf("OP_TERMINATOR");
                break;
            case OP_SEPARATOR:
                printf("OP_SEPARATOR");
                break;
            case OP_AND:
                printf("OP_AND");
                break;
            case OP_OR:
                printf("OP_OR");
                break;
            case OP_PIPE:
                printf("OP_PIPE");
                break;
            default:
                printf("OP_INVALID");
                break;
        }

        printf("\n");
    }
}




