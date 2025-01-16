#ifndef TP1_TOKENIZER_H
#define TP1_TOKENIZER_H

enum token_category {
    TOK_INVALID = 0,
    TOK_SYMBOL,
    TOK_STRING_LITERAL,
    TOK_SEMICOLON,
    TOK_PIPE,
    TOK_LOGICAL_AND,
    TOK_LOGICAL_OR,
    TOK_NEWLINE
};

struct token {
    struct token *next; // Token suivant
    char *value; // Valeur du token, NULL si le token n'a pas de valeur
    enum token_category category; // Catégorie du token
};

/**
 * Cette fonction lit le prochain token depuis l'entrée standard.
 *
 * @return le prochain token ou NULL si la fin de l'entrée est atteinte
 */
struct token* tok_next(void);

/**
 * Cette fonction lit une ligne de tokens depuis l'entrée standard.
 * Une ligne se termine par un token de catégorie TOK_NEWLINE ou NULL si la fin de l'entrée est atteinte.
 *
 * @return la liste de tokens ou NULL si la fin de l'entrée est atteinte
 */
struct token* tok_next_line(void);

/**
 * Cette fonction libère la mémoire allouée pour une liste de tokens.
 *
 * @param tokens list chainée de tokens
 */
void tok_free(struct token* tokens);

/**
 * Cette fonction affiche une liste de tokens.
 * Utilisé pour le débogage.
 *
 * @param tokens list chainée de tokens
 */
void tok_debug_print(const struct token* tokens);



#endif
