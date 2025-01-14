#ifndef LEXER_H_
#define LEXER_H_

#include <stddef.h>
#include <stdio.h>  // Add this line to include the FILE type

typedef enum {
    IDENTIFIER,
    NUM_CONST,
    BOOL_CONST,
    CHAR_CONST,
    FLOAT_CONST,
    STR_CONST,
    STR_WITH_FORMAT,

    FORMAT_INT,
    FORMAT_CHAR,
    FORMAT_FLOAT,
    FORMAT_STR,

    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    INTDIV_OP,
    MOD_OP,
    EXPO_OP,

    REL_LT,
    REL_GT,
    REL_LE,
    REL_GE,
    REL_EQ,
    REL_NEQ,

    LOG_AND,
    LOG_OR,
    LOG_NOT,

    UNARY_INC,
    UNARY_DEC,

    ASSIGN_OP,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    INTDIV_ASSIGN,
    MOD_ASSIGN,

    COMMA,
    SEMICOLON,

    LEFT_CURLY,
    RIGHT_CURLY,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACKET,
    RIGHT_BRACKET,

    KW_BREAK,
    KW_CONTINUE,
    KW_DEFAULT,
    KW_DISPLAY,
    KW_ELSE,
    KW_FOR,
    KW_IF,
    KW_INPUT,
    KW_MAIN,
    KW_RETURN,
    KW_WHILE,

    TYPE_BOOLEAN,
    TYPE_CHARACTER,
    TYPE_FLOAT,
    TYPE_INTEGER,
    TYPE_STRING,

    RW_CONSTANT,
    RW_NULL,
    RW_VOID,

    NW_DO,
    NW_END,
    NW_LET,
    NW_THEN,
    
    TOKEN_COMMENT,
    TOKEN_UNKNOWN,
    TOKEN_INVALID,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    size_t line_num;
} Token;

void print_token(const Token *token);
Token **lexer(FILE *file, size_t *token_count);
void write_to_symbol_table(const Token *token, FILE *symbol_table_file);

#endif // LEXER_H_