#include "lexers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

size_t line_number = 1;        // Global line number

Token *create_token(TokenType type, const char *value, size_t line_num) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->value = strdup(value);
    token->line_num = line_num;
    return token;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        
        case IDENTIFIER: return "IDENTIFIER";
        case NUM_CONST: return "NUM_CONST";
        case BOOL_CONST: return "BOOLEAN";     
        case CHAR_CONST: return "CHAR_CONST";
        case FLOAT_CONST: return "FLOAT_CONST";     
        case STR_CONST: return "STR_CONST";
        case STR_WITH_FORMAT: return "STR_WITH_FORMAT";
        
        case FORMAT_INT: return "FORMAT_INT";
        case FORMAT_CHAR: return "FORMAT_CHAR";
        case FORMAT_FLOAT: return "FORMAT_FLOAT";
        case FORMAT_STR: return "FORMAT_STR";
        
        case ADD_OP: return "ADD_OP";
        case SUB_OP: return "SUB_OP";
        case MUL_OP: return "MUL_OP";
        case DIV_OP: return "DIV_OP";
        case INTDIV_OP: return "INTDIV_OP";
        case MOD_OP: return "MOD_OP";
        case EXPO_OP: return "EXPO_OP";

        case REL_LT: return "REL_LT";
        case REL_GT: return "REL_GT";
        case REL_LE: return "REL_LE";
        case REL_GE: return "REL_GE";
        case REL_EQ: return "REL_EQ";
        case REL_NEQ: return "REL_NEQ";
        
        case LOG_AND: return "LOG_AND";
        case LOG_OR: return "LOG_OR";
        case LOG_NOT: return "LOG_NOT";
        
        case UNARY_INC: return "UNARY_INC";
        case UNARY_DEC: return "UNARY_DEC";

        case ASSIGN_OP: return "ASSIGN_OP";
        case ADD_ASSIGN: return "ADD_ASSIGN";
        case SUB_ASSIGN: return "SUB_ASSIGN";
        case MUL_ASSIGN: return "MUL_ASSIGN";
        case DIV_ASSIGN: return "DIV_ASSIGN";
        case INTDIV_ASSIGN: return "INTDIV_ASSIGN";
        case MOD_ASSIGN: return "MOD_ASSIGN";
        
        case COMMA: return "COMMA";
        case SEMICOLON: return "SEMICOLON";

        case LEFT_CURLY: return "LEFT_CURLY";
        case RIGHT_CURLY: return "RIGHT_CURLY";
        case LEFT_PAREN:return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case LEFT_BRACKET: return "LEFT_BRACKET";
        case RIGHT_BRACKET: return "RIGHT_BRACKET";

        case KW_BREAK: return "KW_BREAK";
        case KW_CONTINUE: return "KW_CONTINUE";
        case KW_DEFAULT: return "KW_DEFAULT";
        case KW_DISPLAY: return "KW_DISPLAY";
        case KW_ELSE: return "KW_ELSE";
        case KW_FOR: return "KW_FOR";
        case KW_IF: return "KW_IF";
        case KW_INPUT: return "KW_INPUT";
        case KW_MAIN: return "KW_MAIN";
        case KW_RETURN: return "KW_RETURN";
        case KW_WHILE: return "KW_WHILE";

        case TYPE_BOOLEAN: return "TYPE_BOOLEAN";
        case TYPE_CHARACTER: return "TYPE_CHARACTER";
        case TYPE_FLOAT: return "TYPE_FLOAT";
        case TYPE_INTEGER: return "TYPE_INTEGER";
        case TYPE_STRING: return "TYPE_STRING";

        case RW_CONSTANT: return "RW_CONSTANT";
        case RW_NULL: return "RW_NULL";
        case RW_VOID: return "RW_VOID";

        case NW_DO: return "NW_DO";
        case NW_END: return "NW_END";
        case NW_LET: return "NW_LET";
        case NW_THEN: return "NW_THEN";

        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        case TOKEN_INVALID: return "INVALID";
        case TOKEN_EOF: return "EOF";
        default: return "UNDEFINED";
    }
}

void print_token(const Token *token) {
    printf("TOKEN: %-20s | TYPE: %-30s | LINE: %zu\n", 
           token->value, 
           token_type_to_string(token->type), 
           token->line_num);
}

void write_to_symbol_table(const Token *token, FILE *symbol_table_file) {
    fprintf(symbol_table_file, "TOKEN: %-20s | TYPE: %-30s | LINE: %zu\n", 
            token->value, 
            token_type_to_string(token->type), 
           token->line_num);
}

void free_token(Token *token) {
    if (token->value) free(token->value);
    free(token);
}

Token *classify_number(const char *source, int *index) {
    char buffer[64] = {0};
    int buffer_index = 0;
    int has_decimal = 0;
    int is_flagged = 0;

    // Append numbers with one decimal point
    while (isdigit(source[*index]) || (source[*index] == '.') || (isalpha(source[*index]) || ispunct(source[*index]))) {
        if (source[*index] == '.') {
            has_decimal++;
        } else if (isalpha(source[*index]) || strchr("@#", source[*index])){ // Flags for containing letters and special characters
            is_flagged++;
        } else if (ispunct(source[*index]) && !(strchr("@#", source[*index]))){ // Exits loop if an operator is seen
            break;
        }
        
        buffer[buffer_index++] = source[(*index)++];
    }

    // Determines if the number of decimals is invalid or has letters
    if (has_decimal > 1 || is_flagged){
        fprintf(stderr, "Error: Invalid token '%s' at line %zu\n", buffer, line_number);
        return create_token(TOKEN_INVALID, buffer, line_number);
    }

    // Determine the token type based on the presence of a decimal point
    TokenType type = has_decimal ? FLOAT_CONST : NUM_CONST;

    return create_token(type, buffer, line_number);
}


Token *classify_string(const char *source, int *index) {
    char buffer[256] = {0};
    int buffer_index = 0;
    int expect_format = 0;
    int format_spec_count = 0;

    (*index)++; // Skip the opening quote

    while (source[*index] != '"' && source[*index] != '\0') {
        
        if (expect_format != 0) {
            if (strchr("dcfs", source[*index])) { // int char float str
                format_spec_count++;
            }

            buffer[buffer_index++] = source[(*index)++];
            expect_format = 0;
            continue;

        } else if (source[*index] == '\n') {
            line_number++;
             
        } else if (source[*index] == '%') {
            buffer[buffer_index++] = source[(*index)++];
            expect_format++;
            continue;
        }
        
        buffer[buffer_index++] = source[(*index)++];
    }

    if (source[*index] == '"') {
        (*index)++; // Skip
    } else {
        fprintf(stderr, "Error: Unterminated string at line %zu\n", line_number);
    }

    if (format_spec_count > 0) {
        return create_token(STR_WITH_FORMAT, buffer, line_number);
    } else {
        return create_token(STR_CONST, buffer, line_number);
    }
    
}


Token *classify_character(const char *source, int *index) {
    (*index)++; // Skip apostrophe
    char c = source[*index];

    // Checks if character is empty 
    if (c == '\0' || c == '\'') {
        fprintf(stderr, "Error: Invalid or empty character at line %zu\n", line_number);
        return NULL;
    }

    (*index)++;

    // Check if there's more than one character inside apostrophe
    if (source[*index] != '\'') {
        char buffer[50];
        int buffer_index = 0;
        (*index)--; // Move back one character

        fprintf(stderr, "Error: Too many characters in character constant at line %zu\n", line_number);

        // Appends characters to buffer
        while (source[*index] != '\'' && source[*index] != '\0') {
            buffer[buffer_index] = source[*index];
            (*index)++;
            buffer_index++;
        }

        if (source[*index] == '\'') { // Skip apostrophe
            (*index)++;
        }

        return create_token(TOKEN_INVALID, buffer, line_number);
    }

    (*index)++;

    char buffer[2] = {c, '\0'};
    return create_token(CHAR_CONST, buffer, line_number);
}


Token *classify_comment(const char *source, int *index) {
    char buffer[256] = {0}; 
    int buffer_index = 0;
    size_t start_line = line_number;  

    // Single-line comment: starts with `~~`
    if (source[*index] == '~' && source[*index + 1] == '~') {
        *index += 2;  // Skip the `~~`
        while (source[*index] != '\n' && source[*index] != '\0') {
            buffer[buffer_index++] = source[(*index)++];
        }
        return create_token(TOKEN_COMMENT, buffer, line_number);  // single-line comment
    }

    // Multi-line comment: starts with `~^` and ends with `^~`
    if (source[*index] == '~' && source[*index + 1] == '^') {
        *index += 2;  // Skip the `~^`
        while (!(source[*index] == '^' && source[*index + 1] == '~') && source[*index] != '\0') {
            if (source[*index] == '\n') {
                line_number++;  // Increment the line number for each new line
                buffer[buffer_index++] = ' ';  // Add a space instead of a newline
            } else {
                buffer[buffer_index++] = source[*index];  // Add other characters normally
            }
            (*index)++;
        }

        // Ensure we skip the closing `^~` if found
        if (source[*index] == '^' && source[*index + 1] == '~') {
            *index += 2;
        } else {
            printf("Warning: Unterminated multi-line comment at line %zu\n", line_number);
        }

        return create_token(TOKEN_COMMENT, buffer, start_line);
    }

    return NULL;  // Not a comment
}


Token *classify_word(const char *lexeme) {
    int startIdx = 0;

    switch (lexeme[startIdx]) {
        case 'b':
            switch (lexeme[startIdx + 1]) {
                case 'r':
                    if (lexeme[startIdx + 2] == 'e' && lexeme[startIdx + 3] == 'a' && lexeme[startIdx + 4] == 'k' && 
                    lexeme[startIdx + 5] == '\0') {
                        return create_token(KW_BREAK, "BREAK", line_number); // "break"
                    }
                    break;
                case 'o':
                    if (lexeme[startIdx + 2] == 'o' && lexeme[startIdx + 3] == 'l' && lexeme[startIdx + 4] == 'e' && 
                    lexeme[startIdx + 5] == 'a' && lexeme[startIdx + 6] == 'n' && lexeme[startIdx + 7] == '\0') {
                        return create_token(TYPE_BOOLEAN, "BOOLEAN", line_number); // "boolean"
                    }
                    break;
            }
            break;
        
        case 'c': // Handles words starting with 'c'
            switch (lexeme[startIdx + 1]) {
                case 'h': // "character"
                    if (lexeme[startIdx + 2] == 'a' && lexeme[startIdx + 3] == 'r' &&
                        lexeme[startIdx + 4] == 'a' && lexeme[startIdx + 5] == 'c' &&
                        lexeme[startIdx + 6] == 't' && lexeme[startIdx + 7] == 'e' &&
                        lexeme[startIdx + 8] == 'r' && lexeme[startIdx + 9] == '\0') {
                        return create_token(TYPE_CHARACTER, "CHARACTER", line_number); // "character"
                    }
                    break;
                case 'o': 
                    switch (lexeme[startIdx + 2]) {
                        case 'n': 
                            switch (lexeme[startIdx + 3]) {
                                case 's': 
                                    if (lexeme[startIdx + 4] == 't' && lexeme[startIdx + 5] == 'a' &&
                                        lexeme[startIdx + 6] == 'n' && lexeme[startIdx + 7] == 't' &&
                                        lexeme[startIdx + 8] == '\0') {
                                        return create_token(RW_CONSTANT, "CONSTANT", line_number); // "constant"
                                    }
                                    break;
                                case 't': 
                                    if (lexeme[startIdx + 4] == 'i' && lexeme[startIdx + 5] == 'n' &&
                                        lexeme[startIdx + 6] == 'u' && lexeme[startIdx + 7] == 'e' &&
                                        lexeme[startIdx + 8] == '\0') {
                                        return create_token(KW_CONTINUE, "CONTINUE", line_number);// "continue"
                                    }
                                    break;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 'd':
            switch (lexeme[startIdx + 1]) {
                case 'o':
                    if (lexeme[startIdx + 2] == '\0') {
                            return create_token(NW_DO, "DO", line_number); // "do"
                    }
                    break;
                case 'e':
                    if (lexeme[startIdx + 2] == 'f' && lexeme[startIdx + 3] == 'a' &&
                        lexeme[startIdx + 4] == 'u' && lexeme[startIdx + 5] == 'l' &&
                        lexeme[startIdx + 6] == 't' && lexeme[startIdx + 7] == '\0') {
                            return create_token(KW_DEFAULT, "DEFAULT", line_number); // "default"
                    }
                    break;
                case 'i':
                    if (lexeme[startIdx + 2] == 's' && lexeme[startIdx + 3] == 'p' &&
                        lexeme[startIdx + 4] == 'l' && lexeme[startIdx + 5] == 'a' &&
                        lexeme[startIdx + 6] == 'y' && lexeme[startIdx + 7] == '\0') {
                            return create_token(KW_DISPLAY, "DISPLAY", line_number); // "display"
                    }
                    break;
            }
            break;
        
        case 'e':
            switch (lexeme[startIdx + 1]) {
                case 'l':
                    if (lexeme[startIdx + 2] == 's' && lexeme[startIdx + 3] == 'e' &&
                        lexeme[startIdx + 4] == '\0') {
                            return create_token(KW_ELSE, "ELSE", line_number); // "else"
                    }
                    break;
                case 'n':
                    if (lexeme[startIdx + 2] == 'd' && lexeme[startIdx + 3] == '\0') {
                    return create_token(NW_END, "END", line_number);// "end"
                }
                break;
            }
            break;
        case 'f':
            switch (lexeme[startIdx + 1]) {
                case 'o':
                    if (lexeme[startIdx + 2] == 'r' && lexeme[startIdx + 3] == '\0') {
                        return create_token(KW_FOR, "FOR", line_number); // "for"
                    }
                    break;
                case 'l':
                    if (lexeme[startIdx + 2] == 'o' && lexeme[startIdx + 3] == 'a' && lexeme[startIdx + 4] == 't' && lexeme[startIdx + 5] == '\0') {
                        return create_token(TYPE_FLOAT, "FLOAT", line_number); // "float"
                    }
                    break;
                case 'a':
                    if (lexeme[startIdx + 2] == 'l' && lexeme[startIdx + 3] == 's' && lexeme[startIdx + 4] == 'e' && lexeme[startIdx + 5] == '\0') {
                        return create_token(BOOL_CONST, "FALSE", line_number); // "false"
                    }
                    break;
            }
            break;
        case 'i':
            switch (lexeme[startIdx + 1]) {
                case 'f':
                    if (lexeme[startIdx + 2] == '\0') {
                        return create_token(KW_IF, "IF", line_number); // "if"
                    }
                    break;
                case 'n':
                    switch (lexeme[startIdx + 2]) {
                        case 't':
                            if (lexeme[startIdx + 3] == 'e' && lexeme[startIdx + 4] == 'g' &&
                                lexeme[startIdx + 5] == 'e' && lexeme[startIdx + 6] == 'r' && lexeme[startIdx + 7] == '\0') {
                                return create_token(TYPE_INTEGER, "INTEGER", line_number); // "integer"
                            }
                            break;
                        case 'p':
                            if (lexeme[startIdx + 3] == 'u' && lexeme[startIdx + 4] == 't' &&
                                lexeme[startIdx + 5] == '\0') {
                                return create_token(KW_INPUT, "INPUT", line_number); // "input"
                            }
                            break;
                    }
                    
            }
            break;
        
        case 'l':
            switch (lexeme[startIdx + 1]) {
                case 'e':
                    if (lexeme[startIdx + 2] == 't' && lexeme[startIdx + 3] == '\0') {
                            return create_token(NW_LET, "LET", line_number);// "let"
                    }
                    break;
            }
            break;
        
        case 'm':
            switch (lexeme[startIdx + 1]) {
                case 'a':
                    if (lexeme[startIdx + 2] == 'i' && lexeme[startIdx + 3] == 'n' &&
                        lexeme[startIdx + 4] == '\0') {
                            return create_token(KW_MAIN, "MAIN", line_number); // "main"
                    }
                    break;
            }
            break;

        case 'n':
            switch (lexeme[startIdx + 1]) {
                case 'u':
                    if (lexeme[startIdx + 2] == 'l' && lexeme[startIdx + 3] == 'l' &&
                        lexeme[startIdx + 4] == '\0') {
                            return create_token(RW_NULL, "NULL", line_number); // "null"
                    }
                    break;
            }
            break;
        
        case 'r':
            switch (lexeme[startIdx + 1]) {
                case 'e':
                    if (lexeme[startIdx + 2] == 't' && lexeme[startIdx + 3] == 'u' &&
                        lexeme[startIdx + 4] == 'r' && lexeme[startIdx + 5] == 'n' &&
                        lexeme[startIdx + 6] == '\0') {
                            return create_token(KW_RETURN, "RETURN", line_number); // "return"
                    }
                    break;
            }
            break;
        
        case 's':
            switch (lexeme[startIdx + 1]) {
                case 't':
                    if (lexeme[startIdx + 2] == 'r' && lexeme[startIdx + 3] == 'i' &&
                        lexeme[startIdx + 4] == 'n' && lexeme[startIdx + 5] == 'g' &&
                        lexeme[startIdx + 6] == '\0') {
                            return create_token(TYPE_STRING, "STRING", line_number); // "string"
                    }
                    break;
            }
            break;
        case 't':
            switch (lexeme[startIdx + 1]) {
                case 'h':
                    if (lexeme[startIdx + 2] == 'e' && lexeme[startIdx + 3] == 'n' &&
                        lexeme[startIdx + 4] == '\0') {
                            return create_token(NW_THEN, "THEN", line_number); // "then"
                    }
                    break;
                case 'r':
                    if (lexeme[startIdx + 2] == 'u' && lexeme[startIdx + 3] == 'e' &&
                        lexeme[startIdx + 4] == '\0') {
                            return create_token(BOOL_CONST, "TRUE", line_number); // "true"
                    }
                    break;
            }
            break;
        
        case 'v':
            switch (lexeme[startIdx + 1]) {
                case 'o':
                    if (lexeme[startIdx + 2] == 'i' && lexeme[startIdx + 3] == 'd' &&
                        lexeme[startIdx + 4] == '\0') {
                            return create_token(RW_VOID, "VOID", line_number);// "void"
                    }
                    break;
            }
            break;
        case 'w':
            switch (lexeme[startIdx + 1]) {
                case 'h':
                    if (lexeme[startIdx + 2] == 'i' && lexeme[startIdx + 3] == 'l' &&
                        lexeme[startIdx + 4] == 'e' &&  lexeme[startIdx + 5] == '\0') {
                            return create_token(KW_WHILE, "WHILE", line_number); // "while"
                    }
                    break;
            }
            break;
        default: 
            break;
    }
    // If no keyword is matched, classify as an identifier
    return create_token(IDENTIFIER, lexeme, line_number);
}


Token *classify_operator(const char *source, int *index) {
    char current = source[*index];
    char next = source[*index + 1];
    (*index)++;

    switch (current) {
        // Relational Operators
        case '<':
            if (next == '=') {
                (*index)++;
                return create_token(REL_LE, "<=", line_number);
            }
            return create_token(REL_LT, "<", line_number);

        case '>':
            if (next == '=') {
                (*index)++;
                return create_token(REL_GE, ">=", line_number);
            }
            return create_token(REL_GT, ">", line_number);

        case '=':
            if (next == '=') {
                (*index)++;
                return create_token(REL_EQ, "==", line_number);
            }
            return create_token(ASSIGN_OP, "=", line_number);

        case '!':
            if (next == '=') {
                (*index)++;
                return create_token(REL_NEQ, "!=", line_number);
            }
            return create_token(LOG_NOT, "!", line_number);

        // Logical Operators
        case '&':
            if (next == '&') {
                (*index)++;
                return create_token(LOG_AND, "&&", line_number);
            }
            break;

        case '|':
            if (next == '|') {
                (*index)++;
                return create_token(LOG_OR, "||", line_number);
            }
            break;

        // Arithmetic Operators and Unary Operators
        case '+':
            if (next == '=') {
                (*index)++;
                return create_token(ADD_ASSIGN, "+=", line_number);
            }
            if (next == '+') {
                (*index)++;
                return create_token(UNARY_INC, "++", line_number);
            }
            return create_token(ADD_OP, "+", line_number);

        case '-':
            if (next == '=') {
                (*index)++;
                return create_token(SUB_ASSIGN, "-=", line_number);
            }
            if (next == '-') {
                (*index)++;
                return create_token(UNARY_DEC, "--", line_number);
            }
            return create_token(SUB_OP, "-", line_number);

        case '*':
            if (next == '=') {
                (*index)++;
                return create_token(MUL_ASSIGN, "*=", line_number);
            }
            return create_token(MUL_OP, "*", line_number);

        case '/':
            if (next == '=') {
                (*index)++;
                return create_token(DIV_ASSIGN, "/=", line_number);
            }
            return create_token(DIV_OP, "/", line_number);

        case '$':
            if (next == '=') {
                (*index)++;
                return create_token(INTDIV_ASSIGN, "$=", line_number);
            }
            return create_token(INTDIV_OP, "$", line_number);

        case '%':
            if (next == '=') {
                (*index)++;
                return create_token(MOD_ASSIGN, "%=", line_number);
            }
            return create_token(MOD_OP, "%", line_number);

        case '^':
            return create_token(EXPO_OP, "^", line_number);

        // Default case for unknown operators
        default: {
            char unknown[2] = {current, '\0'};
            return create_token(TOKEN_UNKNOWN, unknown, line_number);
        }
    }

    return NULL;
}


Token *classify_delimiter(char c, size_t line_number) {
    switch (c) {
        case ',': return create_token(COMMA, ",", line_number);
        case ';': return create_token(SEMICOLON, ";", line_number);

        // Parentheses
        case '(': return create_token(LEFT_PAREN, "(", line_number);
        case ')': return create_token(RIGHT_PAREN, ")", line_number);

        // Braces
        case '{': return create_token(LEFT_CURLY, "{", line_number);
        case '}': return create_token(RIGHT_CURLY, "}", line_number);

        // Brackets
        case '[': return create_token(LEFT_BRACKET, "[", line_number);
        case ']': return create_token(RIGHT_BRACKET, "]", line_number);

        // Default case for unknown delimiters
        default: {
            char unknown[2] = {c, '\0'};
            return create_token(TOKEN_UNKNOWN, unknown, line_number);
        }
    }
}


Token **tokenize(const char *source, size_t *token_count) {
    size_t capacity = 10;
    Token **tokens = malloc(capacity * sizeof(Token *));
    *token_count = 0;
    int index = 0;
    int length = strlen(source);

    while (index < length) {
        char c = source[index];

        // Skip white spaces and track line numbers
        if (isspace(c)) {
            if (c == '\n') line_number++;
            index++;
            continue;
        }

        Token *token = NULL;

        // Comments
        if (source[index] == '~') {
            token = classify_comment(source, &index);
        }
        // Numbers
        else if (isdigit(c)) {
            token = classify_number(source, &index);
        }
        // Keywords or Identifiers
        else if (isalpha(c) || c == '_') {
            char buffer[64] = {0};
            int buffer_index = 0;
            int is_flagged = 0;

            while (isalnum(source[index]) || ispunct(source[index])) {
                if (source[index] == '_'){
                } else if (strchr("@#.`?", source[index])) {
                    is_flagged++;
                } else if (ispunct(source[index]) && !strchr("@#.`?", source[index])) {
                    break;
                }
                                
                buffer[buffer_index++] = source[index++];
            }

            if (is_flagged){
                fprintf(stderr, "Error: Invalid token '%s' at line %zu\n", buffer, line_number);
                token = create_token(TOKEN_INVALID, buffer, line_number);
            
            } else {
                token = classify_word(buffer); // Classify if string is keyword, reserved word, or noise word
            }
        }
        // Operators
        else if (strchr("+-*/=$%^<>!&|", c)) { 
            token = classify_operator(source, &index);
        }
        // Delimiters
        else if (strchr(";{},()[]", c)) {
            token = classify_delimiter(c, line_number);
            index++; 
        }
        else if (c == '"') { // Detect the start of a string
            token = classify_string(source, &index);
        }
        else if (source[index] == '\'') { // Detect the start of a character
            token = classify_character(source, &index);
        }
        // Handle unrecognized characters
        else if (ispunct(source[index])) {
            token = classify_operator(source, &index);
            fprintf(stderr, "Error: Unrecognized character '%c' at line %zu\n", c, line_number);
        }

        // Store token
        if (token) {
            if (*token_count == capacity) {
                capacity *= 2;
                tokens = realloc(tokens, capacity * sizeof(Token *));
            }
            tokens[(*token_count)++] = token;
        }
    }

    return tokens;
}

Token **lexer(FILE *file, size_t *token_count) {
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    Token **tokens = tokenize(buffer, token_count);
    free(buffer);

    return tokens;
}