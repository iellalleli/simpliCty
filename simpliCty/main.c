#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexers.h"
#include "parser.h"

const char* VALID_EXTENSION = ".cty";
void check_file_type(const char* filename, const char* expectedExtension);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: correct syntax: %s <filename.cty>\n\n", argv[0]);
        exit(1);
    }

    check_file_type(argv[1], VALID_EXTENSION);

    // Open the .cty file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("ERROR: File not found\n");
        exit(1);
    }

    // Open/Create the output file for the symbol table
    FILE *symbol_table = fopen("output/symbol_table.txt", "w");
    if (!symbol_table) {
        printf("ERROR: Unable to create the output file\n");
        fclose(file);
        exit(1);
    }

    // Get tokens from the lexer
    size_t token_count = 0;
    Token **tokens = lexer(file, &token_count);
    fclose(file);

    // Check if lexer returned NULL tokens
    if (!tokens) {
        printf("Error: Lexer failed to process the file\n");
        fclose(symbol_table);
        exit(1);
    }

    // Write tokens to the symbol table
    printf("Tokens generated:\n");
    for (size_t i = 0; i < token_count; i++) {
        print_token(tokens[i]);
        write_to_symbol_table(tokens[i], symbol_table);
    }
    fclose(symbol_table);
    printf("Lexical analysis complete.\n");

    // Run the parser
    printf("\n--- Running Parser ---\n");
    runParser("output/symbol_table.txt");  // We don't need to check return value anymore
    printf("Parsing completed. Check parsed.txt for results.\n");

    // Clean up allocated memory for tokens
    for (size_t j = 0; j < token_count; j++) {
        if (tokens[j]->value) {
            free(tokens[j]->value);
        }
        free(tokens[j]);
    }
    free(tokens);  // Free the token array

    printf("Processing complete.\n");
    return 0;
}

// Only files with .cty extensions are accepted
void check_file_type(const char* filename, const char* expectedExtension){
    const char *dot = strrchr(filename, '.');
    
    // Check if filename has an invalid extension
    if(dot == NULL || strcmp(dot, expectedExtension) != 0){
        fprintf(stderr, "Error: unexpected file type: %s\nExpected file type: <filename.cty>\n", filename);
        exit(1);
    }
}