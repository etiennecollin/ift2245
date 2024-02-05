#include "parser.h"
#include "tokenizer.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

struct command* cmd_parse(struct token* tokens)
{
    struct command sentinel = {NULL, NULL, OP_TERMINATOR};
    struct command *current = &sentinel;

    // TODO

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

