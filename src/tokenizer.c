#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

int is_operator(char c) {
    return c == '&' || c == '|' || c == ';' || c == '\n';
}

int is_terminator(char c) {
    return c == '\0' || c == EOF;
}

int is_symbol_char(char c) {
    return !is_whitespace(c) && !is_operator(c) && !is_terminator(c);
}

char toEscaped(char c) {
    switch (c) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 't': return '\t';
        case 'n': return '\n';
        case 'r': return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        default: return c;
    }
}

void buffer_check_sub(char** buffer, int* buffer_size, int i) {
    if (i < *buffer_size - 1) return;
    *buffer_size = *buffer_size + (*buffer_size >> 1); // Grow buffer by 1.5x
    char* new_buffer = realloc(*buffer, *buffer_size);
    if(!new_buffer) // Reallocation failed
    {
        free(*buffer);
        *buffer = NULL;
    }
    *buffer = new_buffer;
}

char* read_string_literal(char c)
{
    char end = c;

    int buffer_size = 32;
    char* buffer = malloc(buffer_size);

    if (!buffer)  return NULL; // String buffer allocation failed

    int i = 0;

    for(;;)
    {
        c = (char)getchar();
        if(c == end) break; // End of string literal
        if(c == EOF) // Unexpected EOF
        {
            free(buffer);
            return NULL;
        }

        if(c == '\\') // Escape character
        {
            c = (char)getchar();
            char es = toEscaped(c);

            if(c == EOF || es == c) // Invalid escape sequence
            {
                free(buffer);
                return NULL;
            }

            c = es;
        }

        buffer[i++] = c;
        buffer_check_sub(&buffer, &buffer_size, i);
        if(buffer == NULL) return NULL; // Grow buffer failed
    }

    buffer[i] = '\0';
    return buffer;
}

char* read_symbol(char c)
{
    if(!is_symbol_char(c)) return NULL; // No symbol start character

    int buffer_size = 64;
    char* buffer = malloc(buffer_size);

    if (!buffer)  return NULL; // String buffer allocation failed

    int i = 1;
    buffer[0] = c;

    for(;;)
    {
        c = (char)getchar();
        if(!is_symbol_char(c))
        {
            ungetc(c, stdin);
            break;
        }

        buffer[i++] = c;
        buffer_check_sub(&buffer, &buffer_size, i);
        if(buffer == NULL) return NULL; // Grow buffer failed
    }

    buffer[i] = '\0';
    return buffer;
}


struct token* tok_next(void)
{
    // Skip whitespace
    signed char c;
    while (is_whitespace(c = (signed char)getchar()));

    // End of input
    if (c == EOF) return NULL;

    // Create token

    struct token* token = malloc(sizeof(struct token));
    memset(token, 0, sizeof(struct token)); // Zero initialize

    switch (c) {
        case '\n': token->category = TOK_NEWLINE; break;
        case ';': token->category = TOK_SEMICOLON; break;
        case '|':
        {
            signed char peek = (signed char) getchar();
            if(peek == '|') token->category = TOK_LOGICAL_OR;
            else { token->category = TOK_PIPE; ungetc(peek, stdin); }
            break;
        }
        case '&':
        {
            signed char peek = (signed char) getchar();
            if(peek == '&') token->category = TOK_LOGICAL_AND;
            else ungetc(peek, stdin);
            break;
        }
        case '\"': // Fallthrough
        case '\'':
        {
            token->category = TOK_STRING_LITERAL;
            token->value = read_string_literal(c);

            if(!token->value) // Failed to read string literal
            {
                free(token);
                return NULL;
            }

            break;
        }
        default: // Symbol
        {
            token->category = TOK_SYMBOL;
            token->value = read_symbol(c);

            if(!token->value) // Failed to read symbol
            {
                free(token);
                return NULL;
            }

            break;
        }
    }

    return token;
}

struct token* tok_next_line(void)
{
    struct token token_sentinel = {NULL, NULL, TOK_INVALID};
    struct token *tokens = &token_sentinel;

    struct token *token;
    while((token = tok_next()))
    {
        tokens->next = token;
        tokens = token;

        if(token->category == TOK_NEWLINE) break;
    }

    return token_sentinel.next;
}

void tok_free(struct token *tokens) {
    for(struct token *token = tokens, *next; token; token = next) {
        next = token->next;
        free(token->value);
        free(token);
    }
}

void tok_debug_print(const struct token *tokens) {
    for(const struct token* token = tokens; token; token = token->next)
    {
        switch (token->category) {
            case TOK_SYMBOL: printf("TOK_SYMBOL(%s)\n", token->value); break;
            case TOK_STRING_LITERAL: printf("TOK_STRING_LITERAL(%s)\n", token->value); break;
            case TOK_PIPE: printf("TOK_PIPE\n"); break;
            case TOK_LOGICAL_AND: printf("TOK_LOGICAL_AND\n"); break;
            case TOK_LOGICAL_OR: printf("TOK_LOGICAL_OR\n"); break;
            case TOK_SEMICOLON: printf("TOK_SEMICOLON\n"); break;
            case TOK_NEWLINE: printf("TOK_NEWLINE\n"); break;
            default: printf("TOK_INVALID\n"); break;
        }
    }
}

