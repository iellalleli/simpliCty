#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexers.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error: correct syntax: %s <filename.cty>\n", argv[0]);
        exit(1);
    }

    // Open the .cty file provided as a command-line argument
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("ERROR: File not found\n");
        exit(1);
    }

    // Get tokens from the lexer
    size_t token_count = 0;
    Token **tokens = lexer(file, &token_count);
    fclose(file);

    // Check if lexer returned NULL tokens
    if (!tokens) {
        printf("Error: Lexer failed to process the file\n");
        exit(1);
    }

    // Print all tokens
    printf("Tokens generated:\n");
    for (size_t i = 0; i < token_count; i++) {
        print_token(tokens[i]);
    }

    // Clean up allocated memory for tokens
    for (size_t j = 0; j < token_count; j++) {
        if (tokens[j]->value) {
            free(tokens[j]->value);
        }
        free(tokens[j]);
    }
    free(tokens);  // Free the token array

    printf("Lexical analysis complete.\n");
    return 0;
}
