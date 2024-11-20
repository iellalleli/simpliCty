#ifndef LEXER_H_
#define LEXER_H_

#include <stddef.h>
#include <stdio.h> 

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_RESERVED_WORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_ARITHMETIC_OPERATOR,
    TOKEN_BOOLEAN_OPERATOR_RELATIONAL,  
    TOKEN_BOOLEAN_OPERATOR_LOGICAL,     
    TOKEN_ASSIGNMENT_OPERATOR,
    TOKEN_UNARY_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_DELIMITER_OPEN_PARENTHESIS,
    TOKEN_DELIMITER_CLOSE_PARENTHESIS,
    TOKEN_DELIMITER_OPEN_BRACE,
    TOKEN_DELIMITER_CLOSE_BRACE,
    TOKEN_DELIMITER_OPEN_BRACKET,
    TOKEN_DELIMITER_CLOSE_BRACKET,
    TOKEN_STRING,
    TOKEN_CHARACTER,
    TOKEN_NOISE_WORD,
    TOKEN_COMMENT,
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
