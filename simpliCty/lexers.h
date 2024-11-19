#ifndef LEXER_H_
#define LEXER_H_

#include <stddef.h>
#include <stdio.h>  // Add this line to include the FILE type

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_ARITHMETIC_OPERATOR,
    TOKEN_RELATIONAL_OPERATOR,
    TOKEN_LOGICAL_OPERATOR,
    TOKEN_ASSIGNMENT_OPERATOR,
    TOKEN_UNARY_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_STRING,
    TOKEN_UNKNOWN,
    TOKEN_EOF
} TokenType;


typedef struct {
    TokenType type;
    char *value;
    size_t line_num;
} Token;


void print_token(const Token *token);
Token **lexer(FILE *file, size_t *token_count);

#endif // LEXER_H_
