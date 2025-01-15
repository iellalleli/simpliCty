#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexers.h"

typedef struct TreeNode {
    int id;
    int parentID;
    char *value;
    struct TreeNode **children;
    size_t childCount;
} TreeNode;

typedef struct {
    char *type;
    char *value;
} TokenInfo;

static size_t nextNodeID = 0;
static size_t currentTokenIndex = 0;
TokenInfo *tokens = NULL;
size_t token_count = 0;
FILE* parsed_file = NULL; 

// Function prototypes
TreeNode* parseSimplicity(); //1
TreeNode* parseDeclStmt(); // 2
TreeNode* parseVarDecl(); //3
TreeNode* parseTypeSpec(); // 4
TreeNode* parseIdList(); // 5
TreeNode* parseAssign(); // 6
TreeNode* parseBoolExp(); // 7
TreeNode* parseBoolTerm(); // 8
TreeNode* parseBoolFactor(); // 9
TreeNode* parseRelExp(); // 10
TreeNode* parseArithExp(); // 11
TreeNode* parseAddMinOp(); // 12
TreeNode* parseTerm(); // 13
TreeNode* parseFactor(); // 14
TreeNode* parseBase(); // 15
TreeNode* parseUpdate(); // 16
TreeNode* parseUpdateOp(); // 17
TreeNode* parseMulDivOp(); // 18
TreeNode* parseRelOp(); // 19
TreeNode* parseBoolLiteral(); // 20
TreeNode* parseAssignment(); // 21
TreeNode* parseArrDecl(); // 22
TreeNode* parseFuncDecl(); // 23
TreeNode* parseParamList(); // 24
TreeNode* parseParam(); // 25
TreeNode* parseFuncStmt(); // 26
TreeNode* parseFuncCall(); // 27
TreeNode* parseArgList(); // 28
TreeNode* parseExp(); // 29
TreeNode* parseFuncDef(); // 30
TreeNode* parseBlock(); // 31
TreeNode* parseStmtList(); // 32
TreeNode* parseStmt(); // 33
TreeNode* parseAssignStmt(); // 34
TreeNode* parseArrStmt(); // 35
TreeNode* parseArrAssign(); // 36
TreeNode* parseArrAccess(); // 37
TreeNode* parseArrInit(); // 38
TreeNode* parseArrList(); // 39
TreeNode* parseArrElem(); // 40
TreeNode* parseCondStmt(); // 41
TreeNode* parseIfStmt(); // 42
TreeNode* parseIfElseStmt(); // 43
TreeNode* parseElseIf(); // 44
TreeNode* parseElse(); // 45
TreeNode* parseIterStmt(); // 46
TreeNode* parseWhileStmt(); // 47
TreeNode* parseForStmt(); // 48
TreeNode* parseReturnStmt(); // 49
TreeNode* parseOutputStmt(); // 50
TreeNode* parseStdOutput(); // 51
TreeNode* parseValueOutput(); // 52
TreeNode* parseFormatSpecifier(); // 53
TreeNode* parseSequenceOutput(); // 54
TreeNode* parseOutputElem(); // 55
TreeNode* parseInputStmt(); // 56

// Utility functions
int match(const char* expectedType);
TreeNode* createNode(const char* value);
void addChild(TreeNode* parent, TreeNode* child);
void freeTree(TreeNode* node);
void writeParseTree(FILE* file, TreeNode* node);
void writeParsingState();
TokenInfo* readSymbolTable(const char* filename, size_t* token_count);

// Function to read tokens from the symbol table
TokenInfo* readSymbolTable(const char* filename, size_t* token_count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening symbol table file\n");
        return NULL;
    }

    TokenInfo* tokens = malloc(10 * sizeof(TokenInfo));
    *token_count = 0;
    size_t capacity = 10;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (*token_count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(TokenInfo));
        }

        char type[64], value[64];
        if (sscanf(line, "TOKEN: %s | TYPE: %s", value, type) == 2) {
            tokens[*token_count].type = strdup(type);
            tokens[*token_count].value = strdup(value);
            (*token_count)++;
        }
    }

    fclose(file);
    return tokens;
}

// Function to write current parsing state
void writeParsingState() {
    if (!parsed_file) return;

    // First line with all types
    for (size_t i = 0; i < token_count; i++) {
        fprintf(parsed_file, "%s ", tokens[i].type);
    }
    fprintf(parsed_file, "\n");

    // Following lines with progressive replacement of types with values
    for (size_t i = 0; i <= currentTokenIndex; i++) {
        for (size_t j = 0; j < token_count; j++) {
            if (j < i) {
                fprintf(parsed_file, "%s ", tokens[j].value);
            } else {
                fprintf(parsed_file, "%s ", tokens[j].type);
            }
        }
        fprintf(parsed_file, "\n");
    }
}

// Match the current token with the expected type and advance if successful
int match(const char *expectedType) {
    if (currentTokenIndex < token_count) {
        printf("Matching token: %s (expected: %s)\n", tokens[currentTokenIndex].type, expectedType);
        if (strcmp(tokens[currentTokenIndex].type, expectedType) == 0) {
            currentTokenIndex++;
            writeParsingState();  // Write state after each successful match
            return 1;
        }
    }
    return 0;
}

// Parse tree management
TreeNode* createNode(const char* value) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->id = nextNodeID++;
    node->parentID = -1;
    node->value = strdup(value);
    node->children = NULL;
    node->childCount = 0;
    return node;
}

void addChild(TreeNode* parent, TreeNode* child) {
    if (!child) return;
    parent->children = realloc(parent->children, sizeof(TreeNode*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
    child->parentID = parent->id;
}

void freeTree(TreeNode* node) {
    if (!node) return;
    for (size_t i = 0; i < node->childCount; i++) {
        freeTree(node->children[i]);
    }
    free(node->value);
    free(node->children);
    free(node);
}

void writeParseTree(FILE* file, TreeNode* node) {
    if (!node) return;

    fprintf(file, "%d,%d,%s\n", node->id, node->parentID, node->value);
    for (size_t i = 0; i < node->childCount; i++) {
        writeParseTree(file, node->children[i]);
    }
}

TreeNode* parseSimplicity() {
    TreeNode* root = createNode("SIMPLICITY");
    
    // [ { DECL_STMT | FUNC_STMT | ARR_STMT } ]
    while (currentTokenIndex < token_count) {
        // Look ahead to determine which type of statement we have
        if (match("RW_CONSTANT") || match("NW_LET") || 
            tokens[currentTokenIndex].type == "TYPE_BOOLEAN" ||
            tokens[currentTokenIndex].type == "TYPE_CHARACTER" ||
            tokens[currentTokenIndex].type == "TYPE_FLOAT" ||
            tokens[currentTokenIndex].type == "TYPE_INTEGER" ||
            tokens[currentTokenIndex].type == "TYPE_STRING" ||
            tokens[currentTokenIndex].type == "RW_VOID") {
            
            size_t savedIndex = currentTokenIndex;
            TreeNode* stmt = NULL;
            
            // Try parsing each type of statement
            if ((stmt = parseDeclStmt()) != NULL) {
                addChild(root, stmt);
            } else {
                currentTokenIndex = savedIndex;
                if ((stmt = parseFuncStmt()) != NULL) {
                    addChild(root, stmt);
                } else {
                    currentTokenIndex = savedIndex;
                    if ((stmt = parseArrStmt()) != NULL) {
                        addChild(root, stmt);
                    } else {
                        // If none of the statements match, break the loop
                        currentTokenIndex = savedIndex;
                        break;
                    }
                }
            }
        } else {
            break;
        }
    }
    
    // TYPE_SPEC
    TreeNode* typeSpec = parseTypeSpec();
    if (!typeSpec) {
        printf("Error: Expected type specifier\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, typeSpec);
    
    // KW_MAIN
    if (!match("KW_MAIN")) {
        printf("Error: Expected 'main'\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, createNode("MAIN"));
    
    // LEFT_PAREN
    if (!match("LEFT_PAREN")) {
        printf("Error: Expected '('\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, createNode("LEFT_PAREN"));
    
    // [ ARG_LIST | RW_VOID ]
    if (match("RW_VOID")) {
        addChild(root, createNode("RW_VOID"));
    } else {
        TreeNode* args = parseArgList();
        if (args) {
            addChild(root, args);
        }
    }
    
    // RIGHT_PAREN
    if (!match("RIGHT_PAREN")) {
        printf("Error: Expected ')'\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, createNode("RIGHT_PAREN"));
    
    // BLOCK
    TreeNode* block = parseBlock();
    if (!block) {
        printf("Error: Expected block\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, block);
    
    return root;
}


// Modified parseTypeSpec to return TreeNode* instead of int
TreeNode* parseTypeSpec() {
    if (match("TYPE_BOOLEAN")) return createNode("TYPE_BOOLEAN");
    if (match("TYPE_CHARACTER")) return createNode("TYPE_CHARACTER");
    if (match("TYPE_FLOAT")) return createNode("TYPE_FLOAT");
    if (match("TYPE_INTEGER")) return createNode("TYPE_INTEGER");
    if (match("TYPE_STRING")) return createNode("TYPE_STRING");
    return NULL;
}

// Modified parseBlock function
TreeNode* parseBlock() {
    TreeNode* block = createNode("BLOCK");
    
    if (!match("LEFT_CURLY")) {  // Changed from LEFT_BRACE to LEFT_CURLY
        printf("Error: Expected LEFT_CURLY\n");
        freeTree(block);
        return NULL;
    }
    addChild(block, createNode("LEFT_CURLY"));

    if (!match("RIGHT_CURLY")) {  // Changed from RIGHT_BRACE to RIGHT_CURLY
        printf("Error: Expected RIGHT_CURLY\n");
        freeTree(block);
        return NULL;
    }
    addChild(block, createNode("RIGHT_CURLY"));

    return block;
}

// Modified parseVarDecl function
TreeNode* parseVarDecl() {
    TreeNode* varDecl = createNode("VAR_DECL");

    if (!match("NW_LET")) {
        printf("Error: Expected NW_LET in variable declaration\n");
        freeTree(varDecl);
        return NULL;
    }
    addChild(varDecl, createNode("NW_LET"));

    // Parse type specifier
    TreeNode* typeSpec = parseTypeSpec();
    if (!typeSpec) {
        printf("Error: Expected type specifier in variable declaration\n");
        freeTree(varDecl);
        return NULL;
    }
    addChild(varDecl, typeSpec);

    if (!match("IDENTIFIER")) {
        printf("Error: Expected identifier in variable declaration\n");
        freeTree(varDecl);
        return NULL;
    }
    addChild(varDecl, createNode(tokens[currentTokenIndex - 1].value));

    if (!match("SEMICOLON")) {
        printf("Error: Expected semicolon in variable declaration\n");
        freeTree(varDecl);
        return NULL;
    }
    addChild(varDecl, createNode("SEMICOLON"));

    return varDecl;
}

// Template for new parsing functions (add your implementations gradually)
TreeNode* parseArrDecl() {
    TreeNode* node = createNode("ARR_DECL");
    // TODO: Implement array declaration parsing
    return node;
}

TreeNode* parseFuncDecl() {
    TreeNode* node = createNode("FUNC_DECL");
    // TODO: Implement function declaration parsing
    return node;
}

TreeNode* parseIdList() {
    TreeNode* node = createNode("ID_LIST");
    // TODO: Implement identifier list parsing
    return node;
}

TreeNode* parseAssign() {
    TreeNode* node = createNode("ASSIGN");
    // TODO: Implement assignment parsing
    return node;
}

// Add all other function templates similarly...


// Main function
void runParser(const char* symbol_table_file) {
    tokens = readSymbolTable(symbol_table_file, &token_count);
    if (!tokens) {
        fprintf(stderr, "Failed to read symbol table\n");
        return;
    }

    // Open parsed.txt for writing
    parsed_file = fopen("output/parsed.txt", "w");
    if (!parsed_file) {
        fprintf(stderr, "Failed to open output/parsed.txt for writing\n");
        // Clean up and return
        for (size_t i = 0; i < token_count; i++) {
            free(tokens[i].type);
            free(tokens[i].value);
        }
        free(tokens);
        return;
    }

    currentTokenIndex = 0;
    nextNodeID = 0;
    TreeNode* parseTree = parseSimplicity();

    if (parseTree) {
        printf("Parsing successful!\n");

        FILE* treeFile = fopen("output/parse_tree.csv", "w");
        if (treeFile) {
            fprintf(treeFile, "NodeID,ParentID,Value\n");
            writeParseTree(treeFile, parseTree);
            fclose(treeFile);
        } else {
            printf("Error: Could not open output/parse_tree.csv for writing\n");
        }
        freeTree(parseTree);
    } else {
        printf("Parsing failed at token %zu: %s\n", currentTokenIndex, 
               currentTokenIndex < token_count ? tokens[currentTokenIndex].type : "END");
    }

    // Close parsed.txt
    if (parsed_file) {
        fclose(parsed_file);
    }

    // Clean up tokens
    for (size_t i = 0; i < token_count; i++) {
        free(tokens[i].type);
        free(tokens[i].value);
    }
    free(tokens);
}
