#include "parser.h"
#include "tokenizer.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/**
 *
 * Determines whether a token is an argument
 * @param category the category of the token
 * @return true if the token parsed is an argument for a command. False otherwise.
 */
int isArg(enum token_category category) {
    if (category == TOK_SEMICOLON ||
        category == TOK_PIPE ||
        category == TOK_LOGICAL_AND ||
        category == TOK_LOGICAL_OR)
        return 0;
    else return 1;
}

/**
 *
 * Counts the number of arguments in a command to determine how much memory to allocate
 * @param tokens points to a token that is the first argument of a command
 * @return the number of arguments in a command
 */
int countArgs(struct token* tokens) {
    struct token *current = tokens;

    int ctr = 0; // counter
    while (current != NULL && isArg(current->category)) {
        ctr++;
        current = current->next;
    }

    return ctr;
}

/**
 *
 * @return the op corresponding to the token category or -1 if error
 */
int findOP(enum token_category category) {
    switch(category) {
        case TOK_SEMICOLON:
            return OP_SEPARATOR;
        case TOK_PIPE:
            return OP_PIPE;
        case TOK_LOGICAL_AND:
            return OP_AND;
        case TOK_LOGICAL_OR:
            return TOK_LOGICAL_OR;
        case TOK_NEWLINE:
            return OP_TERMINATOR; // TODO is this okay?
        default: // handles tok invalid, tok symbol, tok string literal
            return -1;
    }
}

/**
 *
 * @param tokens points to the first token in the linked list of tokens
 * @return A linked list of commands
 */
struct command* cmd_parse(struct token* tokens) {
    // TODO : should we create a dummy token pointer to iterate through the token's linked list without losing the token pointer?

    struct command sentinel = {NULL, NULL, OP_TERMINATOR}; // head of linked list
    struct command *current = &sentinel;

    while (tokens != NULL) {
        // initialize newCmd
        struct command *newCmd = (struct command*) malloc(sizeof(struct command));
        newCmd->next = NULL;
        newCmd->args = NULL;
        newCmd->op = OP_TERMINATOR;

        // determine number of args of command
        int numOfArgs = countArgs(tokens);

        // allocate memory for command args
        newCmd->args = (char **) malloc(sizeof(char **) * (numOfArgs));

        // build new command
        for (int i = 0; i < numOfArgs; i++) {
            newCmd->args[i] = tokens->value;
            tokens = tokens->next; // next token to be parsed
        }

        // token is a separator or NULL
        newCmd->op = findOP(tokens->category);

        current->next = newCmd; // add the new command to the linked list
        current = newCmd;
    }
    return sentinel.next;
}

void cmd_free(struct command *command)
{
    // TODO
}

void cmd_debug_print(const struct command* commands)
{
    for(const struct command* cmd = commands; cmd; cmd = cmd->next)
    {
        for(int i = 0; cmd->args[i]; i++)
        {
            printf("%s ", cmd->args[i]);
        }

        switch (cmd->op) {
            case OP_TERMINATOR: printf("OP_TERMINATOR"); break;
            case OP_SEPARATOR: printf("OP_SEPARATOR"); break;
            case OP_AND: printf("OP_AND"); break;
            case OP_OR: printf("OP_OR"); break;
            case OP_PIPE: printf("OP_PIPE"); break;
            default: printf("OP_INVALID"); break;
        }

        printf("\n");
    }
}



