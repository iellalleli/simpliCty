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
TreeNode* parseElseIfStmt(); // 44
TreeNode* parseElseStmt(); // 45
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
int match(const char* expectedType, int isOptional);
TreeNode* createNode(const char* value);
void addChild(TreeNode* parent, TreeNode* child);
void freeTree(TreeNode* node);
void writeParseTree(FILE* file, TreeNode* node);
void writeParseTreeParenthesized(FILE* file, TreeNode* node, int depth);
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
        if (strstr(line, "TOKEN:") && strstr(line, "TYPE:")) {
            if (*token_count >= capacity) {
                capacity *= 2;
                tokens = realloc(tokens, capacity * sizeof(TokenInfo));
            }

            // Tokenize the line by splitting around "TOKEN:" and "TYPE:"
            char* tokenPart = strtok(line, "|");
            char* typePart = strtok(NULL, "|");

            // Process the "TOKEN" value
            if (tokenPart) {
                char value[100];
                if (sscanf(tokenPart, "TOKEN: %s", value) == 1) {
                    tokens[*token_count].value = strdup(value);
                }
            }

            // Process the "TYPE" value
            if (typePart) {
                char type[100];
                if (sscanf(typePart, " TYPE: %s", type) == 1) {
                    tokens[*token_count].type = strdup(type);
                }
            }

            // Print the token and its type
            printf("%s %s ", tokens[*token_count].value, tokens[*token_count].type);
            (*token_count)++;
        }
    }

    fclose(file);
    return tokens;
}

// Function to write current parsing state
void writeParsingState() {
    if (!parsed_file) return;

    // Write the current state of the tokens to parsed.txt
    for (size_t i = 0; i < token_count; i++) {
        if (i < currentTokenIndex) {
            // Replace type with value for already parsed tokens
            fprintf(parsed_file, "%s ", tokens[i].value);
        } else {
            // Keep type for unparsed tokens
            fprintf(parsed_file, "%s ", tokens[i].type);
        }
    }
    fprintf(parsed_file, "\n");
}


// Match the current token with the expected type and advance if successful
int match(const char *expectedType, int isOptional) {
    if (currentTokenIndex < token_count) {
        printf("Matching token: %s (expected: %s)\n", tokens[currentTokenIndex].type, expectedType);

        // If the current token matches the expected type, proceed
        if (strcmp(tokens[currentTokenIndex].type, expectedType) == 0) {
            currentTokenIndex++;
            writeParsingState();  // Write state after each successful match
            return 1;
        }

        // Log and skip if optional
        if (isOptional) {
            printf("Optional token '%s' not found. Skipping...\n", expectedType);
            return 0;
        }

        // Otherwise, return failure for mandatory tokens
        printf("Unexpected token: %s (expected: %s). Stopping.\n", 
               tokens[currentTokenIndex].type, expectedType);
        return 0;
    }

    printf("No more tokens to match (expected: %s)\n", expectedType);
    return 0;
}


// Parse tree management
TreeNode* createNode(const char* value) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->value = strdup(value);
    node->childCount = 0;
    node->children = NULL;
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
    for (int i = 0; i < node->childCount; i++) {
        freeTree(node->children[i]);
    }
    free(node->children);
    free(node->value);
    free(node);
}


void writeParseTree(FILE* file, TreeNode* node) {
    if (!node) return;

    fprintf(file, "%d,%d,%s\n", node->id, node->parentID, node->value);
    for (size_t i = 0; i < node->childCount; i++) {
        writeParseTree(file, node->children[i]);
    }
}


// Parsing functions
TreeNode* parseSimplicity() {
    TreeNode* root = createNode("SIMPLICITY");

    // [ { DECL_STMT | FUNC_STMT | ARR_STMT } ]
    while (currentTokenIndex < token_count) {
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
    if (!match("KW_MAIN",0)) {
        printf("Error: Expected 'main'\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, createNode("MAIN"));

    // LEFT_PAREN
    if (!match("LEFT_PAREN",0)) {
        printf("Error: Expected '('\n");
        freeTree(root);
        return NULL;
    }
    addChild(root, createNode("LEFT_PAREN"));

    // [ ARG_LIST | RW_VOID ]
    if (match("RW_VOID",1)) {
        addChild(root, createNode("RW_VOID"));
    } else {
        TreeNode* args = parseArgList();
        if (args) {
            addChild(root, args);
        }
    }

    // RIGHT_PAREN
    if (!match("RIGHT_PAREN",0)) {
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

TreeNode* parseDeclStmt() {
    TreeNode* declStmt = createNode("DECL_STMT");

    // Save the current token index to backtrack if needed
    size_t savedIndex = currentTokenIndex;

    // Attempt to parse a variable declaration
    TreeNode* varDecl = parseVarDecl();
    if (varDecl) {
        addChild(declStmt, varDecl);
        return declStmt;
    }
    currentTokenIndex = savedIndex;

    // Attempt to parse an array declaration
    TreeNode* arrDecl = parseArrDecl();
    if (arrDecl) {
        addChild(declStmt, arrDecl);
        return declStmt;
    }
    currentTokenIndex = savedIndex;

    // Attempt to parse a function declaration
    TreeNode* funcDecl = parseFuncDecl();
    if (funcDecl) {
        addChild(declStmt, funcDecl);
        return declStmt;
    }

    // If none of the declarations match, return NULL
    freeTree(declStmt);
    return NULL;
}

TreeNode* parseVarDecl() {
    TreeNode* varDecl = createNode("VAR_DECL");

    // Optional RW_CONSTANT
    match("RW_CONSTANT", 1); // Pass '1' to indicate optional

    // Optional NW_LET
    match("NW_LET", 1); // Pass '1' to indicate optional

    // TYPE_SPEC (mandatory)
    TreeNode* typeSpec = parseTypeSpec();
    if (!typeSpec) {
        printf("Error: Expected TYPE_SPEC\n");
        freeTree(varDecl); // Cleanup if TYPE_SPEC is missing
        return NULL;
    }
    addChild(varDecl, typeSpec);

    // ID_LIST or IDENTIFIER (mandatory)
    TreeNode* idList = parseIdList();
    if (idList) {
        addChild(varDecl, idList);
    } else if (match("IDENTIFIER", 0)) {
        addChild(varDecl, createNode("IDENTIFIER"));
    } else {
        printf("Error: Expected ID_LIST or IDENTIFIER\n");
        freeTree(varDecl); // Cleanup if neither ID_LIST nor IDENTIFIER is present
        return NULL;
    }

    // SEMICOLON (mandatory)
    if (!match("SEMICOLON", 0)) {
        printf("Error: Expected SEMICOLON\n");
        freeTree(varDecl); // Cleanup if SEMICOLON is missing
        return NULL;
    }
    addChild(varDecl, createNode("SEMICOLON"));

    return varDecl;
}

TreeNode* parseTypeSpec() {
    if (match("TYPE_BOOLEAN",0)) return createNode("TYPE_BOOLEAN");
    if (match("TYPE_CHARACTER",0)) return createNode("TYPE_CHARACTER");
    if (match("TYPE_FLOAT",0)) return createNode("TYPE_FLOAT");
    if (match("TYPE_INTEGER",0)) return createNode("TYPE_INTEGER");
    if (match("TYPE_STRING",0)) return createNode("TYPE_STRING");
    return NULL;
}

TreeNode* parseIdList() {
    TreeNode* idList = createNode("ID_LIST");

    // Parse the first IDENTIFIER
    if (!match("IDENTIFIER",0)) {
        freeTree(idList); // Cleanup if no IDENTIFIER is found
        return NULL;
    }
    addChild(idList, createNode("IDENTIFIER"));

    // Optionally parse ASSIGN after the first IDENTIFIER
    TreeNode* assign = parseAssign();
    if (assign) {
        addChild(idList, assign);
    }

    // Parse zero or more {COMMA IDENTIFIER [ASSIGN]}
    while (match("COMMA",0)) {
        TreeNode* commaNode = createNode("COMMA");
        addChild(idList, commaNode);

        // Ensure an IDENTIFIER follows the COMMA
        if (!match("IDENTIFIER",0)) {
            freeTree(idList); // Cleanup if no IDENTIFIER follows COMMA
            return NULL;
        }
        addChild(idList, createNode("IDENTIFIER"));

        // Optionally parse ASSIGN after this IDENTIFIER
        assign = parseAssign();
        if (assign) {
            addChild(idList, assign);
        }
    }

    return idList;
}

TreeNode* parseAssign() {
    TreeNode* assign = createNode("ASSIGN");

    // Ensure the ASSIGN_OP token is present
    if (!match("ASSIGN_OP",0)) {
        freeTree(assign); // Cleanup if ASSIGN_OP is missing
        return NULL;
    }
    addChild(assign, createNode("ASSIGN_OP"));

    // Check for RW_NULL
    if (match("RW_NULL",0)) {
        addChild(assign, createNode("RW_NULL"));
        return assign;
    }

    // Check for STR_CONST
    if (match("STR_CONST",0)) {
        addChild(assign, createNode("STR_CONST"));
        return assign;
    }

    // Check for CHAR_CONST
    if (match("CHAR_CONST",0)) {
        addChild(assign, createNode("CHAR_CONST"));
        return assign;
    }

    // Check for ASSIGNMENT followed by ARITH_EXP
    TreeNode* assignment = parseAssignment();
    if (assignment) {
        addChild(assign, assignment);

        TreeNode* arithExp = parseArithExp();
        if (arithExp) {
            addChild(assign, arithExp);
            return assign;
        } else {
            // Cleanup if ARITH_EXP is missing
            freeTree(assign);
            // return NULL;
        }
    }

    // Check for BOOL_EXP
    TreeNode* boolExp = parseBoolExp();
    if (boolExp) {
        addChild(assign, boolExp);
        return assign;
    }

    // If none of the options matched, cleanup and return NULL
    // freeTree(assign);
    return assign;
}

TreeNode* parseBoolExp() {
    // Create the root node for BOOL_EXP
    TreeNode* boolExp = createNode("BOOL_EXP");

    // Attempt to parse the first BOOL_TERM
    TreeNode* boolTerm = parseBoolTerm();
    if (!boolTerm) {
        freeTree(boolExp); // Cleanup if BOOL_TERM is missing
        return NULL;
    }
    addChild(boolExp, boolTerm);

    // Check for repeated LOG_OR BOOL_TERM (right-recursive rule)
    while (match("LOG_OR",0)) {
        // Add the LOG_OR operator as a child
        addChild(boolExp, createNode("LOG_OR"));

        // Parse the next BOOL_TERM
        TreeNode* nextBoolTerm = parseBoolTerm();
        if (!nextBoolTerm) {
            freeTree(boolExp); // Cleanup if BOOL_TERM is missing after LOG_OR
            return NULL;
        }
        addChild(boolExp, nextBoolTerm);
    }

    return boolExp;
}

TreeNode* parseBoolTerm() {
    // Create the root node for BOOL_TERM
    TreeNode* boolTerm = createNode("BOOL_TERM");

    // Attempt to parse the first BOOL_FACTOR
    TreeNode* boolFactor = parseBoolFactor();
    if (!boolFactor) {
        freeTree(boolTerm); // Cleanup if BOOL_FACTOR is missing
        return NULL;
    }
    addChild(boolTerm, boolFactor);

    // Check for repeated LOG_AND BOOL_FACTOR (right-recursive rule)
    while (match("LOG_AND",0)) {
        // Add the LOG_AND operator as a child
        addChild(boolTerm, createNode("LOG_AND"));

        // Parse the next BOOL_FACTOR
        TreeNode* nextBoolFactor = parseBoolFactor();
        if (!nextBoolFactor) {
            freeTree(boolTerm); // Cleanup if BOOL_FACTOR is missing after LOG_AND
            return NULL;
        }
        addChild(boolTerm, nextBoolFactor);
    }

    return boolTerm;
}

TreeNode* parseBoolFactor() {
    
    printf("================================== BOOLFACTOR");
    // Create the root node for BOOL_FACTOR
    TreeNode* boolFactor = createNode("BOOL_FACTOR");

    // Case 1: LOG_NOT BOOL_FACTOR
    if (match("LOG_NOT",0)) {
        addChild(boolFactor, createNode("LOG_NOT"));

        TreeNode* nextBoolFactor = parseBoolFactor();
        if (nextBoolFactor) {
            addChild(boolFactor, nextBoolFactor);
            return boolFactor;
        } else {
            // Cleanup if BOOL_FACTOR is missing after LOG_NOT
            freeTree(boolFactor);
            return NULL;
        }
    }

    // Case 2: REL_EXP
    TreeNode* relExp = parseRelExp();
    if (relExp) {
        addChild(boolFactor, relExp);
        return boolFactor;
    }

    printf("================================== BACK TO BOOLFACT");
    // Case 3: LEFT_PAREN BOOL_EXP RIGHT_PAREN
    if (match("LEFT_PAREN",0)) {
        addChild(boolFactor, createNode("LEFT_PAREN"));

        TreeNode* boolExp = parseBoolExp();
        if (!boolExp) {
            // Cleanup if BOOL_EXP is missing after LEFT_PAREN
            freeTree(boolFactor);
            return NULL;
        }
        addChild(boolFactor, boolExp);

        if (!match("RIGHT_PAREN",0)) {
            // Cleanup if RIGHT_PAREN is missing
            freeTree(boolFactor);
            return NULL;
        }
        addChild(boolFactor, createNode("RIGHT_PAREN"));
        return boolFactor;
    }

    printf("================================== BACK TO BOOLFACT");
    // Case 4: ARITH_EXP
    TreeNode* arithExp = parseArithExp();
    if (arithExp) {
        addChild(boolFactor, arithExp);
        return boolFactor;
    }

    // Case 5: BOOL_LITERAL
    if (match("BOOL_LITERAL",0)) {
        addChild(boolFactor, createNode("BOOL_LITERAL"));
        return boolFactor;
    }

    // Case 6: IDENTIFIER
    if (match("IDENTIFIER",0)) {
        addChild(boolFactor, createNode("IDENTIFIER"));
        return boolFactor;
    }

    // If none of the options match, cleanup and return NULL
    freeTree(boolFactor);
    return NULL;
}

TreeNode* parseRelExp() {
    
    printf("================================== RELEXP");
    // Create the root node for REL_EXP
    TreeNode* relExp = createNode("REL_EXP");

    // Attempt to parse the first ARITH_EXP
    TreeNode* firstArithExp = parseArithExp();
    if (firstArithExp) {
        // freeTree(relExp); // Cleanup if the first ARITH_EXP is missing
        // return NULL;
        addChild(relExp, firstArithExp);
    }

    printf("================================== REL_OP");
    // Attempt to parse REL_OP
    TreeNode* relOp = parseRelOp();
    
    if (!relOp) {
        freeTree(relExp); // Cleanup if REL_OP is missing
        return NULL;
    }
    addChild(relExp, relOp);

    printf("================================== 2ND ARITH");
    // Attempt to parse the second ARITH_EXP
    TreeNode* secondArithExp = parseArithExp();
    
    if (!secondArithExp) {
        freeTree(relExp); // Cleanup if the second ARITH_EXP is missing
        return NULL;
    }
    addChild(relExp, secondArithExp);

    return relExp;
}

TreeNode* parseArithExp() { 
    
    printf("================================== ARITH");
    // Create the root node for ARITH_EXP
    TreeNode* arithExp = createNode("ARITH_EXP");

    // Attempt to parse the first TERM
    TreeNode* term = parseTerm();
    if (!term) {
        freeTree(arithExp); // Cleanup if TERM is missing
        return NULL;
    }
    addChild(arithExp, term);
    printf("Done with first term ===================");
    // Parse { ADDMIN_OP TERM }
    
    while (1) {
        printf("Done with first term ===================");
        // Check if ADDMIN_OP is present
        TreeNode* addMinOp = parseAddMinOp();
        if (!addMinOp) {
            break; // Exit the loop if no ADDMIN_OP is found
        }

        // Add ADDMIN_OP to the tree
        addChild(arithExp, addMinOp);

        // Parse the next TERM
        TreeNode* nextTerm = parseTerm();
        if (!nextTerm) {
            freeTree(arithExp); // Cleanup if TERM is missing after ADDMIN_OP
            return NULL;
        }
        addChild(arithExp, nextTerm);
    }

    return arithExp;
}

TreeNode* parseAddMinOp() {
    if (match("ADD_OP",0)) return createNode("ADD_OP");
    if (match("SUB_OP",0)) return createNode("SUB_OP"); 
    return NULL;
}

TreeNode* parseTerm() {

    printf("================================== TERM");
    // Create the root node for TERM
    TreeNode* term = createNode("TERM");

    // Attempt to parse the first FACTOR
    TreeNode* factor = parseFactor();
    if (!factor) {
        freeTree(term); // Cleanup if FACTOR is missing
        return NULL;
    }
    addChild(term, factor);

    
    printf("===================================== done first TERM");
    // Parse { MULDIV_OP FACTOR }
    while (1) {
        // Check if MULDIV_OP is present
        TreeNode* mulDivOp = parseMulDivOp();
        if (!mulDivOp) {
            break; // Exit the loop if no MULDIV_OP is found
        }

        // Add MULDIV_OP to the tree
        addChild(term, mulDivOp);

        // Parse the next FACTOR
        TreeNode* nextFactor = parseFactor();
        if (!nextFactor) {
            freeTree(term); // Cleanup if FACTOR is missing after MULDIV_OP
            return NULL;
        }
        addChild(term, nextFactor);
    }

    return term;
}

TreeNode* parseFactor() {
    printf("================================== FACTOR");
    // Create the root node for FACTOR
    TreeNode* factor = createNode("FACTOR");

    // Attempt to parse the BASE
    TreeNode* base = parseBase();
    if (!base) {
        freeTree(factor); // Cleanup if BASE is missing
        return NULL;
    }
    addChild(factor, base);


    printf("================================== done parse base");
    // Parse { EXPO_OP FACTOR }
    while (1) {
        // Check if EXPO_OP (terminal) is present
        if (!match("EXPO_OP",1)) {
            break; // Exit the loop if no EXPO_OP is found
        }

        // Add EXPO_OP to the tree
        TreeNode* expoOp = createNode("EXPO_OP");
        addChild(factor, expoOp);

        // Parse the next FACTOR
        TreeNode* nextFactor = parseFactor();
        if (!nextFactor) {
            freeTree(factor); // Cleanup if FACTOR is missing after EXPO_OP
            return NULL;
        }
        addChild(factor, nextFactor);
    }

    return factor;
}

TreeNode* parseBase() { 
    printf("================================== BASE");
    // Create the root node for BASE
    TreeNode* base = createNode("BASE");

    // Handle the case: LEFT_PAREN ARITH_EXP RIGHT_PAREN
    if (match("LEFT_PAREN",0)) {
        TreeNode* leftParen = createNode("LEFT_PAREN");
        addChild(base, leftParen);

        TreeNode* arithExp = parseArithExp();
        if (!arithExp) {
            freeTree(base); // Cleanup if ARITH_EXP is missing
            return NULL;
        }
        addChild(base, arithExp);

        if (!match("RIGHT_PAREN",0)) {
            freeTree(base); // Cleanup if RIGHT_PAREN is missing
            return NULL;
        }
        TreeNode* rightParen = createNode("RIGHT_PAREN");
        addChild(base, rightParen);

        return base;
    }

    // Handle the case: UPDATE
    TreeNode* update = parseUpdate();
    if (update) {
        addChild(base, update);
        return base;
    }

    // Handle the case: IDENTIFIER
    if (match("IDENTIFIER",0)) {
        TreeNode* identifier = createNode("IDENTIFIER");
        addChild(base, identifier);
        return base;
    }

    // Handle the case: NUM_CONST
    if (match("NUM_CONST",0)) {
        TreeNode* numConst = createNode("NUM_CONST");
        addChild(base, numConst);
        return base;
    }

    // Handle the case: FLOAT_CONST
    if (match("FLOAT_CONST",0)) {
        TreeNode* floatConst = createNode("FLOAT_CONST");
        addChild(base, floatConst);
        return base;
    }

    // If none of the cases match, return NULL
    freeTree(base);
    return NULL;
}

TreeNode* parseUpdate() { 
    // Create the root node for UPDATE
    TreeNode* update = createNode("UPDATE");

    // Handle the case: IDENTIFIER UPDATE_OP
    if (match("IDENTIFIER",0)) {
        TreeNode* identifier = createNode("IDENTIFIER");
        addChild(update, identifier);

        TreeNode* updateOp = parseUpdateOp();
        if (!updateOp) {
            freeTree(update); // Cleanup if UPDATE_OP is missing
            return NULL;
        }
        addChild(update, updateOp);
        return update;
    }

    // Handle the case: UPDATE_OP IDENTIFIER
    TreeNode* updateOp = parseUpdateOp();
    if (updateOp) {
        addChild(update, updateOp);

        if (match("IDENTIFIER",0)) {
            TreeNode* identifier = createNode("IDENTIFIER");
            addChild(update, identifier);
            return update;
        }
    }

    // If neither case matches, cleanup and return NULL
    freeTree(update);
    return NULL;
}

TreeNode* parseUpdateOp() {
    // Create the root node for UPDATE_OP
    TreeNode* updateOp = createNode("UPDATE_OP");

    // Check for UNARY_INC
    if (match("UNARY_INC",0)) {
        TreeNode* unaryInc = createNode("UNARY_INC");
        addChild(updateOp, unaryInc);
        return updateOp;
    }

    // Check for UNARY_DEC
    if (match("UNARY_DEC",0)) {
        TreeNode* unaryDec = createNode("UNARY_DEC");
        addChild(updateOp, unaryDec);
        return updateOp;
    }

    // If neither token matches, cleanup and return NULL
    freeTree(updateOp);
    return NULL;
}

TreeNode* parseMulDivOp() {
    // Create the root node for MULDIV_OP
    TreeNode* mulDivOp = createNode("MULDIV_OP");

    // Check for each possible operator
    if (match("MUL_OP",0)) {
        TreeNode* mulOp = createNode("MUL_OP");
        addChild(mulDivOp, mulOp);
        return mulDivOp;
    }
    if (match("DIV_OP",0)) {
        TreeNode* divOp = createNode("DIV_OP");
        addChild(mulDivOp, divOp);
        return mulDivOp;
    }
    if (match("INTDIV_OP",0)) {
        TreeNode* intDivOp = createNode("INTDIV_OP");
        addChild(mulDivOp, intDivOp);
        return mulDivOp;
    }
    if (match("MOD_OP",0)) {
        TreeNode* modOp = createNode("MOD_OP");
        addChild(mulDivOp, modOp);
        return mulDivOp;
    }

    // If no match, cleanup and return NULL
    freeTree(mulDivOp);
    return NULL;
}

TreeNode* parseRelOp() {
    // Create the root node for REL_OP
    TreeNode* relOp = createNode("REL_OP");

    // Check for each possible relational operator
    if (match("REL_LT",0)) {
        TreeNode* relLt = createNode("REL_LT");
        addChild(relOp, relLt);
        return relOp;
    }
    if (match("REL_GT",0)) {
        TreeNode* relGt = createNode("REL_GT");
        addChild(relOp, relGt);
        return relOp;
    }
    if (match("REL_LE",0)) {
        TreeNode* relLe = createNode("REL_LE");
        addChild(relOp, relLe);
        return relOp;
    }
    if (match("REL_GE",0)) {
        TreeNode* relGe = createNode("REL_GE");
        addChild(relOp, relGe);
        return relOp;
    }
    if (match("REL_EQ",0)) {
        TreeNode* relEq = createNode("REL_EQ");
        addChild(relOp, relEq);
        return relOp;
    }
    if (match("REL_NEQ",0)) {
        TreeNode* relNeq = createNode("REL_NEQ");
        addChild(relOp, relNeq);
        return relOp;
    }

    // If no match, cleanup and return NULL
    freeTree(relOp);
    return NULL;
}

TreeNode* parseBoolLiteral() {
    // Create the root node for BOOL_LITERAL
    TreeNode* boolLiteral = createNode("BOOL_LITERAL");

    // Check for BOOL_CONST
    if (match("BOOL_CONST",0)) {
        TreeNode* boolConst = createNode("BOOL_CONST");
        addChild(boolLiteral, boolConst);
        return boolLiteral;
    }

    // If no match, cleanup and return NULL
    freeTree(boolLiteral);
    return NULL;
}

TreeNode* parseAssignment() {
    // Create the root node for ASSIGNMENT
    TreeNode* assignment = createNode("ASSIGNMENT");

    // Check for each possible assignment operator
    if (match("ASSIGN_OP",0)) {
        TreeNode* assignOp = createNode("ASSIGN_OP");
        addChild(assignment, assignOp);
        return assignment;
    }
    if (match("ADD_ASSIGN",0)) {
        TreeNode* addAssign = createNode("ADD_ASSIGN");
        addChild(assignment, addAssign);
        return assignment;
    }
    if (match("SUB_ASSIGN",0)) {
        TreeNode* subAssign = createNode("SUB_ASSIGN");
        addChild(assignment, subAssign);
        return assignment;
    }
    if (match("MUL_ASSIGN",0)) {
        TreeNode* mulAssign = createNode("MUL_ASSIGN");
        addChild(assignment, mulAssign);
        return assignment;
    }
    if (match("DIV_ASSIGN",0)) {
        TreeNode* divAssign = createNode("DIV_ASSIGN");
        addChild(assignment, divAssign);
        return assignment;
    }
    if (match("INTDIV_ASSIGN",0)) {
        TreeNode* intDivAssign = createNode("INTDIV_ASSIGN");
        addChild(assignment, intDivAssign);
        return assignment;
    }
    if (match("MOD_ASSIGN",0)) {
        TreeNode* modAssign = createNode("MOD_ASSIGN");
        addChild(assignment, modAssign);
        return assignment;
    }

    // If no match, cleanup and return NULL
    freeTree(assignment);
    return NULL;
}

TreeNode* parseArrDecl() {
    // Create the root node for ARR_DECL
    TreeNode* arrDecl = createNode("ARR_DECL");

    // Optional RW_CONSTANT
    if (match("RW_CONSTANT",1)) {
        TreeNode* rwConstant = createNode("RW_CONSTANT");
        addChild(arrDecl, rwConstant);
    }

    // Parse TYPE_SPEC
    TreeNode* typeSpec = parseTypeSpec();
    if (!typeSpec) {
        freeTree(arrDecl); // Cleanup if TYPE_SPEC is missing
        return NULL;
    }
    addChild(arrDecl, typeSpec);

    // Parse IDENTIFIER
    if (match("IDENTIFIER",0)) {
        TreeNode* identifier = createNode("IDENTIFIER");
        addChild(arrDecl, identifier);
    } else {
        freeTree(arrDecl); // Cleanup if IDENTIFIER is missing
        return NULL;
    }

    // Parse LEFT_BRACKET
    if (match("LEFT_BRACKET",0)) {
        TreeNode* leftBracket = createNode("LEFT_BRACKET");
        addChild(arrDecl, leftBracket);
    } else {
        freeTree(arrDecl); // Cleanup if LEFT_BRACKET is missing
        return NULL;
    }

    // Parse NUM_CONST
    if (match("NUM_CONST",0)) {
        TreeNode* numConst = createNode("NUM_CONST");
        addChild(arrDecl, numConst);
    } else {
        freeTree(arrDecl); // Cleanup if NUM_CONST is missing
        return NULL;
    }

    // Parse RIGHT_BRACKET
    if (match("RIGHT_BRACKET",0)) {
        TreeNode* rightBracket = createNode("RIGHT_BRACKET");
        addChild(arrDecl, rightBracket);
    } else {
        freeTree(arrDecl); // Cleanup if RIGHT_BRACKET is missing
        return NULL;
    }

    // Parse SEMICOLON
    if (match("SEMICOLON",0)) {
        TreeNode* semicolon = createNode("SEMICOLON");
        addChild(arrDecl, semicolon);
        return arrDecl;
    } else {
        freeTree(arrDecl); // Cleanup if SEMICOLON is missing
        return NULL;
    }
}

TreeNode* parseFuncDecl() {
    // Create the root node for FUNC_DECL
    TreeNode* funcDecl = createNode("FUNC_DECL");

    // Parse TYPE_SPEC or RW_VOID
    if (match("RW_VOID",1)) {
        TreeNode* rwVoid = createNode("RW_VOID");
        addChild(funcDecl, rwVoid);
    } else {
        TreeNode* typeSpec = parseTypeSpec();
        if (!typeSpec) {
            freeTree(funcDecl); // Cleanup if neither RW_VOID nor TYPE_SPEC is found
            return NULL;
        }
        addChild(funcDecl, typeSpec);
    }

    // Parse IDENTIFIER
    if (match("IDENTIFIER",0)) {
        TreeNode* identifier = createNode("IDENTIFIER");
        addChild(funcDecl, identifier);
    } else {
        freeTree(funcDecl); // Cleanup if IDENTIFIER is missing
        return NULL;
    }

    // Parse LEFT_PAREN
    if (match("LEFT_PAREN",0)) {
        TreeNode* leftParen = createNode("LEFT_PAREN");
        addChild(funcDecl, leftParen);
    } else {
        freeTree(funcDecl); // Cleanup if LEFT_PAREN is missing
        return NULL;
    }

    // Parse PARAM_LIST
    TreeNode* paramList = parseParamList();
    if (!paramList) {
        freeTree(funcDecl); // Cleanup if PARAM_LIST is missing
        return NULL;
    }
    addChild(funcDecl, paramList);

    // Parse RIGHT_PAREN
    if (match("RIGHT_PAREN",0)) {
        TreeNode* rightParen = createNode("RIGHT_PAREN");
        addChild(funcDecl, rightParen);
    } else {
        freeTree(funcDecl); // Cleanup if RIGHT_PAREN is missing
        return NULL;
    }

    // Parse SEMICOLON
    if (match("SEMICOLON",0)) {
        TreeNode* semicolon = createNode("SEMICOLON");
        addChild(funcDecl, semicolon);
        return funcDecl; // Successfully parsed FUNC_DECL
    } else {
        freeTree(funcDecl); // Cleanup if SEMICOLON is missing
        return NULL;
    }
}

TreeNode* parseParamList() {
    // Create the root node for PARAM_LIST
    TreeNode* paramList = createNode("PARAM_LIST");

    // Parse the first PARAM
    TreeNode* param = parseParam();
    if (!param) {
        freeTree(paramList); // Cleanup if the first PARAM is missing
        return NULL;
    }
    addChild(paramList, param);

    // Handle the recursive rule: { COMMA PARAM }
    while (match("COMMA",0)) {
        // Add COMMA as a child
        TreeNode* comma = createNode("COMMA");
        addChild(paramList, comma);

        // Parse the next PARAM
        param = parseParam();
        if (!param) {
            freeTree(paramList); // Cleanup if PARAM is missing after a COMMA
            return NULL;
        }
        addChild(paramList, param);
    }

    return paramList; // Successfully parsed PARAM_LIST
}

TreeNode* parseParam() {
    // Create the root node for PARAM
    TreeNode* param = createNode("PARAM");

    // Attempt to parse TYPE_SPEC IDENTIFIER
    TreeNode* typeSpec = parseTypeSpec();
    if (typeSpec) {
        addChild(param, typeSpec);

        if (match("IDENTIFIER",0)) {
            TreeNode* identifier = createNode("IDENTIFIER");
            addChild(param, identifier);
            return param; // Successfully parsed TYPE_SPEC IDENTIFIER
        } else {
            freeTree(param); // Cleanup if IDENTIFIER is missing
            return NULL;
        }
    }

    // Attempt to parse ARR_DECL
    TreeNode* arrDecl = parseArrDecl();
    if (arrDecl) {
        addChild(param, arrDecl);
        return param; // Successfully parsed ARR_DECL
    }

    // If neither rule matches, clean up and return NULL
    freeTree(param);
    return NULL;
}

TreeNode* parseFuncStmt() {
    // Create the root node for FUNC_STMT
    TreeNode* funcStmt = createNode("FUNC_STMT");

    // Attempt to parse FUNC_CALL
    TreeNode* funcCall = parseFuncCall();
    if (funcCall) {
        addChild(funcStmt, funcCall);
        return funcStmt; // Successfully parsed FUNC_CALL
    }

    // Attempt to parse FUNC_DEF
    TreeNode* funcDef = parseFuncDef();
    if (funcDef) {
        addChild(funcStmt, funcDef);
        return funcStmt; // Successfully parsed FUNC_DEF
    }

    // If neither FUNC_CALL nor FUNC_DEF matches, clean up and return NULL
    freeTree(funcStmt);
    return NULL;
}

TreeNode* parseFuncCall() {
    // Create the root node for FUNC_CALL
    TreeNode* funcCall = createNode("FUNC_CALL");

    // Match IDENTIFIER
    if (match("IDENTIFIER",0)) {
        TreeNode* identifier = createNode("IDENTIFIER");
        addChild(funcCall, identifier);

        // Match LEFT_PAREN
        if (match("LEFT_PAREN",0)) {
            TreeNode* leftParen = createNode("LEFT_PAREN");
            addChild(funcCall, leftParen);

            // Parse ARG_LIST
            TreeNode* argList = parseArgList();
            if (argList) {
                addChild(funcCall, argList);

                // Match RIGHT_PAREN
                if (match("RIGHT_PAREN",0)) {
                    TreeNode* rightParen = createNode("RIGHT_PAREN");
                    addChild(funcCall, rightParen);

                    // Match SEMICOLON
                    if (match("SEMICOLON",0)) {
                        TreeNode* semicolon = createNode("SEMICOLON");
                        addChild(funcCall, semicolon);
                        return funcCall; // Successfully parsed FUNC_CALL
                    }
                }
            }
        }
    }

    // If any part fails, clean up and return NULL
    freeTree(funcCall);
    return NULL;
}

TreeNode* parseArgList() {
    // Create the root node for ARG_LIST
    TreeNode* argList = createNode("ARG_LIST");

    // Parse the first EXP
    TreeNode* exp = parseExp();
    if (!exp) {
        freeTree(argList); // Cleanup if no EXP is found
        return NULL;
    }
    addChild(argList, exp);

    // Parse optional { , EXP } sequence
    while (match("COMMA",0)) {
        TreeNode* comma = createNode("COMMA");
        addChild(argList, comma);

        // Parse the next EXP
        exp = parseExp();
        if (!exp) {
            freeTree(argList); // Cleanup if EXP after COMMA is missing
            return NULL;
        }
        addChild(argList, exp);
    }

    return argList; // Successfully parsed ARG_LIST
}

TreeNode* parseExp() {
    // Create the root node for EXP
    TreeNode* exp = createNode("EXP");

    // Attempt to parse ARITH_EXP
    TreeNode* arithExp = parseArithExp();
    if (arithExp) {
        addChild(exp, arithExp);
        return exp; // Successfully parsed ARITH_EXP
    }

    // Attempt to parse BOOL_EXP
    TreeNode* boolExp = parseBoolExp();
    if (boolExp) {
        addChild(exp, boolExp);
        return exp; // Successfully parsed BOOL_EXP
    }

    // If neither ARITH_EXP nor BOOL_EXP matches, clean up and return NULL
    freeTree(exp);
    return NULL;
}

TreeNode* parseFuncDef() {
    // Create the root node for FUNC_DEF
    TreeNode* funcDef = createNode("FUNC_DEF");

    // Parse TYPE_SPEC
    TreeNode* typeSpec = parseTypeSpec();
    if (!typeSpec) {
        freeTree(funcDef);
        return NULL; // TYPE_SPEC is mandatory
    }
    addChild(funcDef, typeSpec);

    // Match IDENTIFIER
    if (!match("IDENTIFIER",0)) {
        freeTree(funcDef);
        return NULL; // IDENTIFIER is mandatory
    }
    TreeNode* identifier = createNode("IDENTIFIER");
    addChild(funcDef, identifier);

    // Match LEFT_PAREN
    if (!match("LEFT_PAREN",0)) {
        freeTree(funcDef);
        return NULL; // LEFT_PAREN is mandatory
    }
    TreeNode* leftParen = createNode("LEFT_PAREN");
    addChild(funcDef, leftParen);

    // Parse PARAM_LIST
    TreeNode* paramList = parseParamList();
    if (!paramList) {
        freeTree(funcDef);
        return NULL; // PARAM_LIST is mandatory
    }
    addChild(funcDef, paramList);

    // Match RIGHT_PAREN
    if (!match("RIGHT_PAREN",0)) {
        freeTree(funcDef);
        return NULL; // RIGHT_PAREN is mandatory
    }
    TreeNode* rightParen = createNode("RIGHT_PAREN");
    addChild(funcDef, rightParen);

    // Parse BLOCK
    TreeNode* block = parseBlock();
    if (!block) {
        freeTree(funcDef);
        return NULL; // BLOCK is mandatory
    }
    addChild(funcDef, block);

    // Return the successfully parsed FUNC_DEF node
    return funcDef;
}

TreeNode* parseBlock() {
    // Create the root node for BLOCK
    TreeNode* block = createNode("BLOCK");

    // Match LEFT_CURLY
    if (!match("LEFT_CURLY",0)) {
        freeTree(block);
        return NULL; // LEFT_CURLY is mandatory
    }
    TreeNode* leftCurly = createNode("LEFT_CURLY");
    addChild(block, leftCurly);

    // Parse STMT_LIST
    TreeNode* stmtList = parseStmtList();
    if (!stmtList) {
        freeTree(block);
        return NULL; // STMT_LIST is mandatory
    }
    addChild(block, stmtList);

    // Parse optional [RETURN_STMT | KW_BREAK | KW_CONTINUE]
    if (match("KW_BREAK",1)) {
        TreeNode* kwBreak = createNode("KW_BREAK");
        addChild(block, kwBreak);
    } else if (match("KW_CONTINUE",1)) {
        TreeNode* kwContinue = createNode("KW_CONTINUE");
        addChild(block, kwContinue);
    } else {
        TreeNode* returnStmt = parseReturnStmt();
        if (returnStmt) {
            addChild(block, returnStmt);
        }
    }

    // Parse optional [NW_END]
    if (match("NW_END",1)) {
        TreeNode* nwEnd = createNode("NW_END");
        addChild(block, nwEnd);
    }

    // Match RIGHT_CURLY
    if (!match("RIGHT_CURLY",0)) {
        freeTree(block);
        return NULL; // RIGHT_CURLY is mandatory
    }
    TreeNode* rightCurly = createNode("RIGHT_CURLY");
    addChild(block, rightCurly);

    // Return the successfully parsed BLOCK node
    return block;
}

TreeNode* parseStmtList() {
    // Create the root node for STMT_LIST
    TreeNode* stmtList = createNode("STMT_LIST");

    // Parse the first mandatory STMT
    TreeNode* stmt = parseStmt();
    if (!stmt) {
        freeTree(stmtList);
        return NULL; // At least one STMT is mandatory
    }
    addChild(stmtList, stmt);

    // Parse zero or more additional STMTs
    while (1) {
        // Try parsing another STMT
        stmt = parseStmt();
        if (!stmt) break; // Stop if no more STMTs are found
        addChild(stmtList, stmt);
    }

    // Return the successfully parsed STMT_LIST node
    return stmtList;
}

TreeNode* parseStmt() {
    // Attempt to parse DECL_STMT
    TreeNode* stmt = parseDeclStmt();
    if (stmt) return stmt;

    // Attempt to parse ASSIGN_STMT
    stmt = parseAssignStmt();
    if (stmt) return stmt;

    // Attempt to parse ARR_STMT
    stmt = parseArrStmt();
    if (stmt) return stmt;

    // Attempt to parse COND_STMT
    stmt = parseCondStmt();
    if (stmt) return stmt;

    // Attempt to parse ITER_STMT
    stmt = parseIterStmt();
    if (stmt) return stmt;

    // Attempt to parse FUNC_STMT
    stmt = parseFuncStmt();
    if (stmt) return stmt;

    // Attempt to parse OUTPUT_STMT
    stmt = parseOutputStmt();
    if (stmt) return stmt;

    // Attempt to parse INPUT_STMT
    stmt = parseInputStmt();
    if (stmt) return stmt;

    // If no valid statement was found, return NULL
    return NULL;
}

TreeNode* parseAssignStmt() {
    printf("===================================== ASSIGN");
    // Create the root node for ASSIGN_STMT
    TreeNode* assignStmt = createNode("ASSIGN_STMT");

    // Optional NW_LET
    if (match("NW_LET",1)) {
        TreeNode* nwLet = createNode("NW_LET");
        addChild(assignStmt, nwLet);
    }


    printf("===================================== MATCH IDENTIFIER");
    // Match IDENTIFIER
    if (!match("IDENTIFIER",0)) {
        freeTree(assignStmt);
        return NULL; // IDENTIFIER is mandatory
    }
    TreeNode* identifier = createNode("IDENTIFIER");
    addChild(assignStmt, identifier);

    printf("===================================== ASSIGN");
    // Parse ASSIGN (nonterminal)
    TreeNode* assign = parseAssign();
    if (!assign) {
        freeTree(assignStmt);
        return NULL; // ASSIGN is mandatory
    }
    addChild(assignStmt, assign);


    printf("===================================== SEMICOLON");
    // Match SEMICOLON
    if (!match("SEMICOLON",0)) {
        freeTree(assignStmt);
        return NULL; // SEMICOLON is mandatory
    }
    TreeNode* semicolon = createNode("SEMICOLON");
    addChild(assignStmt, semicolon);

    // Return the successfully parsed ASSIGN_STMT node
    return assignStmt;
}

TreeNode* parseArrStmt() {
    // Create the root node for ARR_STMT
    TreeNode* arrStmt = createNode("ARR_STMT");

    // Attempt to parse ARR_ASSIGN
    TreeNode* arrAssign = parseArrAssign();
    if (arrAssign) {
        addChild(arrStmt, arrAssign);
        return arrStmt;
    }

    // Attempt to parse ARR_INIT
    TreeNode* arrInit = parseArrInit();
    if (arrInit) {
        addChild(arrStmt, arrInit);
        return arrStmt;
    }

    // If neither ARR_ASSIGN nor ARR_INIT matched, free the node and return NULL
    freeTree(arrStmt);
    return NULL;
}

TreeNode* parseArrAssign() {
    // Create the root node for ARR_ASSIGN
    TreeNode* arrAssign = createNode("ARR_ASSIGN");

    // Parse ARR_ACCESS (nonterminal)
    TreeNode* arrAccess = parseArrAccess();
    if (!arrAccess) {
        freeTree(arrAssign);
        return NULL; // ARR_ACCESS is mandatory
    }
    addChild(arrAssign, arrAccess);

    // Parse ASSIGN (nonterminal)
    TreeNode* assign = parseAssign();
    if (!assign) {
        freeTree(arrAssign);
        return NULL; // ASSIGN is mandatory
    }
    addChild(arrAssign, assign);

    // Match SEMICOLON
    if (!match("SEMICOLON",0)) {
        freeTree(arrAssign);
        return NULL; // SEMICOLON is mandatory
    }
    TreeNode* semicolon = createNode("SEMICOLON");
    addChild(arrAssign, semicolon);

    // Return the successfully parsed ARR_ASSIGN node
    return arrAssign;
}

TreeNode* parseArrAccess() {
    // Create the root node for ARR_ACCESS
    TreeNode* arrAccess = createNode("ARR_ACCESS");

    // Match IDENTIFIER
    if (!match("IDENTIFIER",0)) {
        freeTree(arrAccess);
        return NULL; // IDENTIFIER is mandatory
    }
    TreeNode* identifier = createNode("IDENTIFIER");
    addChild(arrAccess, identifier);

    // Match LEFT_BRACKET
    if (!match("LEFT_BRACKET",0)) {
        freeTree(arrAccess);
        return NULL; // LEFT_BRACKET is mandatory
    }
    TreeNode* leftBracket = createNode("LEFT_BRACKET");
    addChild(arrAccess, leftBracket);

    // Parse ARITH_EXP (nonterminal)
    TreeNode* arithExp = parseArithExp();
    if (!arithExp) {
        freeTree(arrAccess);
        return NULL; // ARITH_EXP is mandatory
    }
    addChild(arrAccess, arithExp);

    // Match RIGHT_BRACKET
    if (!match("RIGHT_BRACKET",0)) {
        freeTree(arrAccess);
        return NULL; // RIGHT_BRACKET is mandatory
    }
    TreeNode* rightBracket = createNode("RIGHT_BRACKET");
    addChild(arrAccess, rightBracket);

    // Return the successfully parsed ARR_ACCESS node
    return arrAccess;
}

TreeNode* parseArrInit() {
    // Create the root node for ARR_INIT
    TreeNode* arrInit = createNode("ARR_INIT");

    // Parse TYPE_SPEC (nonterminal)
    TreeNode* typeSpec = parseTypeSpec();
    if (!typeSpec) {
        freeTree(arrInit);
        return NULL; // TYPE_SPEC is mandatory
    }
    addChild(arrInit, typeSpec);

    // Match IDENTIFIER
    if (!match("IDENTIFIER",0)) {
        freeTree(arrInit);
        return NULL; // IDENTIFIER is mandatory
    }
    TreeNode* identifier = createNode("IDENTIFIER");
    addChild(arrInit, identifier);

    // Match LEFT_BRACKET
    if (!match("LEFT_BRACKET",0)) {
        freeTree(arrInit);
        return NULL; // LEFT_BRACKET is mandatory
    }
    TreeNode* leftBracket1 = createNode("LEFT_BRACKET");
    addChild(arrInit, leftBracket1);

    // Match NUM_CONST
    if (!match("NUM_CONST",0)) {
        freeTree(arrInit);
        return NULL; // NUM_CONST is mandatory
    }
    TreeNode* numConst = createNode("NUM_CONST");
    addChild(arrInit, numConst);

    // Match RIGHT_BRACKET
    if (!match("RIGHT_BRACKET",0)) {
        freeTree(arrInit);
        return NULL; // RIGHT_BRACKET is mandatory
    }
    TreeNode* rightBracket1 = createNode("RIGHT_BRACKET");
    addChild(arrInit, rightBracket1);

    // Match ASSIGN_OP
    if (!match("ASSIGN_OP",0)) {
        freeTree(arrInit);
        return NULL; // ASSIGN_OP is mandatory
    }
    TreeNode* assignOp = createNode("ASSIGN_OP");
    addChild(arrInit, assignOp);

    // Match LEFT_BRACKET
    if (!match("LEFT_BRACKET",0)) {
        freeTree(arrInit);
        return NULL; // LEFT_BRACKET is mandatory
    }
    TreeNode* leftBracket2 = createNode("LEFT_BRACKET");
    addChild(arrInit, leftBracket2);

    // Parse ARR_LIST (nonterminal)
    TreeNode* arrList = parseArrList();
    if (!arrList) {
        freeTree(arrInit);
        return NULL; // ARR_LIST is mandatory
    }
    addChild(arrInit, arrList);

    // Match RIGHT_BRACKET
    if (!match("RIGHT_BRACKET",0)) {
        freeTree(arrInit);
        return NULL; // RIGHT_BRACKET is mandatory
    }
    TreeNode* rightBracket2 = createNode("RIGHT_BRACKET");
    addChild(arrInit, rightBracket2);

    // Match SEMICOLON
    if (!match("SEMICOLON",0)) {
        freeTree(arrInit);
        return NULL; // SEMICOLON is mandatory
    }
    TreeNode* semicolon = createNode("SEMICOLON");
    addChild(arrInit, semicolon);

    // Return the successfully parsed ARR_INIT node
    return arrInit;
}

TreeNode* parseArrList() {
    // Create the root node for ARR_LIST
    TreeNode* arrList = createNode("ARR_LIST");

    // Parse the first ARR_ELEM (mandatory)
    TreeNode* arrElem = parseArrElem();
    if (!arrElem) {
        freeTree(arrList);
        return NULL; // ARR_ELEM is mandatory
    }
    addChild(arrList, arrElem);

    // Parse any additional ARR_ELEM separated by COMMA
    while (match("COMMA",0)) {
        TreeNode* comma = createNode("COMMA");
        addChild(arrList, comma);

        // Parse the next ARR_ELEM
        TreeNode* nextArrElem = parseArrElem();
        if (!nextArrElem) {
            freeTree(arrList);
            return NULL; // If there's a COMMA, ARR_ELEM is mandatory
        }
        addChild(arrList, nextArrElem);
    }

    // Return the successfully parsed ARR_LIST node
    return arrList;
}

TreeNode* parseArrElem() {
    // Try to parse ARR_ACCESS (nonterminal)
    TreeNode* arrAccess = parseArrAccess();
    if (arrAccess) {
        return arrAccess; // If ARR_ACCESS is valid, return it
    }

    // Match BOOL_LITERAL (nonterminal)
    TreeNode* boolLiteral = parseBoolLiteral();
    if (boolLiteral) {
        return boolLiteral; // If BOOL_LITERAL is valid, return it
    }

    // Match IDENTIFIER (terminal)
    if (match("IDENTIFIER",0)) {
        return createNode("IDENTIFIER");
    }

    // Match NUM_CONST (terminal)
    if (match("NUM_CONST",0)) {
        return createNode("NUM_CONST");
    }

    // Match CHAR_CONST (terminal)
    if (match("CHAR_CONST",0)) {
        return createNode("CHAR_CONST");
    }

    // Match STR_CONST (terminal)
    if (match("STR_CONST",0)) {
        return createNode("STR_CONST");
    }

    // Match FLOAT_CONST (terminal)
    if (match("FLOAT_CONST",0)) {
        return createNode("FLOAT_CONST");
    }

    // If none of the cases match, return NULL (parsing failed)
    return NULL;
}

TreeNode* parseCondStmt() {
    // Create the root node for COND_STMT
    TreeNode* condStmt = createNode("COND_STMT");

    // Attempt to parse IF_STMT
    TreeNode* ifStmt = parseIfStmt();
    if (ifStmt) {
        addChild(condStmt, ifStmt);
        return condStmt;
    }

    // Attempt to parse IFELSE_STMT
    TreeNode* ifElseStmt = parseIfElseStmt();
    if (ifElseStmt) {
        addChild(condStmt, ifElseStmt);
        return condStmt;
    }

    // Attempt to parse ELSEIF_STMT
    TreeNode* elseIfStmt = parseElseIfStmt();
    if (elseIfStmt) {
        addChild(condStmt, elseIfStmt);
        return condStmt;
    }

    // If none match, free the root node and return NULL
    freeTree(condStmt);
    return NULL;
}

TreeNode* parseIfStmt() {
    // Create the root node for IF_STMT
    TreeNode* ifStmt = createNode("IF_STMT");

    // Match the KW_IF token
    if (!match("KW_IF",0)) {
        freeTree(ifStmt);
        return NULL; // If "if" keyword is not found, return NULL
    }

    // Match the LEFT_PAREN token
    if (!match("LEFT_PAREN",0)) {
        freeTree(ifStmt);
        return NULL; // If "(" is not found, return NULL
    }

    // Parse BOOL_EXP (nonterminal)
    TreeNode* boolExp = parseBoolExp();
    if (!boolExp) {
        freeTree(ifStmt);
        return NULL; // If BOOL_EXP is not parsed, return NULL
    }
    addChild(ifStmt, boolExp);

    // Match the RIGHT_PAREN token
    if (!match("RIGHT_PAREN",0)) {
        freeTree(ifStmt);
        return NULL; // If ")" is not found, return NULL
    }

    // Optionally match NW_THEN (if present)
    if (match("NW_THEN",1)) {
        TreeNode* thenNode = createNode("NW_THEN");
        addChild(ifStmt, thenNode);
    }

    // Parse BLOCK (nonterminal)
    TreeNode* block = parseBlock();
    if (!block) {
        freeTree(ifStmt);
        return NULL; // If BLOCK is not parsed, return NULL
    }
    addChild(ifStmt, block);

    // Return the successfully parsed IF_STMT node
    return ifStmt;
}

TreeNode* parseIfElseStmt() {
    // Create the root node for IFELSE_STMT
    TreeNode* ifElseStmt = createNode("IFELSE_STMT");

    // Parse IF_STMT (nonterminal)
    TreeNode* ifStmt = parseIfStmt();
    if (!ifStmt) {
        freeTree(ifElseStmt);
        return NULL; // If IF_STMT parsing fails, return NULL
    }
    addChild(ifElseStmt, ifStmt);

    // Parse ELSE_STMT (nonterminal)
    TreeNode* elseStmt = parseElseStmt();
    if (!elseStmt) {
        freeTree(ifElseStmt);
        return NULL; // If ELSE_STMT parsing fails, return NULL
    }
    addChild(ifElseStmt, elseStmt);

    // Return the successfully parsed IFELSE_STMT node
    return ifElseStmt;
}

TreeNode* parseElseIfStmt() {
    // Create the root node for ELSEIF_STMT
    TreeNode* elseIfStmt = createNode("ELSEIF_STMT");

    // Parse the initial IF_STMT (nonterminal)
    TreeNode* ifStmt = parseIfStmt();
    if (!ifStmt) {
        freeTree(elseIfStmt);
        return NULL; // If IF_STMT parsing fails, return NULL
    }
    addChild(elseIfStmt, ifStmt);

    // Parse the sequence of KW_ELSE KW_IF (loop)
    while (match("KW_ELSE",0) && match("KW_IF",0)) {
        // Parse LEFT_PAREN
        if (!match("LEFT_PAREN",0)) {
            freeTree(elseIfStmt);
            return NULL; // If LEFT_PAREN is not found, return NULL
        }

        // Parse BOOL_EXP
        TreeNode* boolExp = parseBoolExp();
        if (!boolExp) {
            freeTree(elseIfStmt);
            return NULL; // If BOOL_EXP parsing fails, return NULL
        }
        addChild(elseIfStmt, boolExp);

        // Parse RIGHT_PAREN
        if (!match("RIGHT_PAREN",0)) {
            freeTree(elseIfStmt);
            return NULL; // If RIGHT_PAREN is not found, return NULL
        }

        // Optionally match NW_THEN
        if (match("NW_THEN",1)) {
            TreeNode* thenNode = createNode("NW_THEN");
            addChild(elseIfStmt, thenNode);
        }

        // Parse BLOCK
        TreeNode* block = parseBlock();
        if (!block) {
            freeTree(elseIfStmt);
            return NULL; // If BLOCK parsing fails, return NULL
        }
        addChild(elseIfStmt, block);
    }

    // Optionally parse ELSE_STMT
    if (match("KW_ELSE",1)) {
        TreeNode* elseStmt = parseElseStmt();
        if (elseStmt) {
            addChild(elseIfStmt, elseStmt);
        }
    }

    // Return the successfully parsed ELSEIF_STMT node
    return elseIfStmt;
}

TreeNode* parseElseStmt() {
    // Create the root node for ELSE_STMT
    TreeNode* elseStmt = createNode("ELSE_STMT");

    // Match KW_ELSE
    if (!match("KW_ELSE",0)) {
        freeTree(elseStmt);
        return NULL; // If KW_ELSE is not found, return NULL
    }

    // Parse BLOCK (nonterminal)
    TreeNode* block = parseBlock();
    if (!block) {
        freeTree(elseStmt);
        return NULL; // If BLOCK parsing fails, return NULL
    }
    addChild(elseStmt, block);

    // Return the successfully parsed ELSE_STMT node
    return elseStmt;
}

TreeNode* parseIterStmt() {
    // Create the root node for ITER_STMT
    TreeNode* iterStmt = createNode("ITER_STMT");

    // Attempt to parse WHILE_STMT
    TreeNode* whileStmt = parseWhileStmt();
    if (whileStmt) {
        addChild(iterStmt, whileStmt);
        return iterStmt;  // If WHILE_STMT parsing is successful, return the node
    }

    // Attempt to parse FOR_STMT
    TreeNode* forStmt = parseForStmt();
    if (forStmt) {
        addChild(iterStmt, forStmt);
        return iterStmt;  // If FOR_STMT parsing is successful, return the node
    }

    // If neither WHILE_STMT nor FOR_STMT is found, free the root and return NULL
    freeTree(iterStmt);
    return NULL;
}

TreeNode* parseWhileStmt() {
    // Create the root node for WHILE_STMT
    TreeNode* whileStmt = createNode("WHILE_STMT");

    // Match KW_WHILE token
    if (!match("KW_WHILE",0)) {
        freeTree(whileStmt);
        return NULL; // If KW_WHILE is not found, return NULL
    }

    // Match LEFT_PAREN token
    if (!match("LEFT_PAREN",0)) {
        freeTree(whileStmt);
        return NULL; // If LEFT_PAREN is not found, return NULL
    }

    // Parse BOOL_EXP (nonterminal)
    TreeNode* boolExp = parseBoolExp();
    if (!boolExp) {
        freeTree(whileStmt);
        return NULL; // If BOOL_EXP parsing fails, return NULL
    }
    addChild(whileStmt, boolExp);

    // Match RIGHT_PAREN token
    if (!match("RIGHT_PAREN",0)) {
        freeTree(whileStmt);
        return NULL; // If RIGHT_PAREN is not found, return NULL
    }

    // Check for optional NW_DO token (do-block syntax)
    match("NW_DO",1);

    // Parse BLOCK (nonterminal)
    TreeNode* block = parseBlock();
    if (!block) {
        freeTree(whileStmt);
        return NULL; // If BLOCK parsing fails, return NULL
    }
    addChild(whileStmt, block);

    // Return the successfully parsed WHILE_STMT node
    return whileStmt;
}

TreeNode* parseForStmt() {
    // Create the root node for FOR_STMT
    TreeNode* forStmt = createNode("FOR_STMT");

    // Match KW_FOR token
    if (!match("KW_FOR",0)) {
        freeTree(forStmt);
        return NULL; // If KW_FOR is not found, return NULL
    }

    // Match LEFT_PAREN token
    if (!match("LEFT_PAREN",0)) {
        freeTree(forStmt);
        return NULL; // If LEFT_PAREN is not found, return NULL
    }

    // Parse either VAR_DECL or ASSIGN_STMT (one of these nonterminals)
    TreeNode* initStmt = parseVarDecl();
    if (!initStmt) {
        initStmt = parseAssignStmt(); // Try ASSIGN_STMT if VAR_DECL fails
    }
    if (!initStmt) {
        freeTree(forStmt);
        return NULL; // If neither VAR_DECL nor ASSIGN_STMT is found, return NULL
    }
    addChild(forStmt, initStmt);  // Add the initStmt to the FOR_STMT node

    // Parse BOOL_EXP (nonterminal)
    TreeNode* boolExp = parseBoolExp();
    if (!boolExp) {
        freeTree(forStmt);
        return NULL; // If BOOL_EXP parsing fails, return NULL
    }
    addChild(forStmt, boolExp);

    // Match SEMICOLON token
    if (!match("SEMICOLON",0)) {
        freeTree(forStmt);
        return NULL; // If SEMICOLON is not found, return NULL
    }

    // Parse UPDATE (nonterminal)
    TreeNode* update = parseUpdate();
    if (!update) {
        freeTree(forStmt);
        return NULL; // If UPDATE parsing fails, return NULL
    }
    addChild(forStmt, update);

    // Match RIGHT_PAREN token
    if (!match("RIGHT_PAREN",0)) {
        freeTree(forStmt);
        return NULL; // If RIGHT_PAREN is not found, return NULL
    }

    // Check for optional NW_DO token (do-block syntax)
    match("NW_DO",1);

    // Parse BLOCK (nonterminal)
    TreeNode* block = parseBlock();
    if (!block) {
        freeTree(forStmt);
        return NULL; // If BLOCK parsing fails, return NULL
    }
    addChild(forStmt, block);

    // Return the successfully parsed FOR_STMT node
    return forStmt;
}

TreeNode* parseReturnStmt() {
    // Create the root node for RETURN_STMT
    TreeNode* returnStmt = createNode("RETURN_STMT");

    // Match KW_RETURN token
    if (!match("KW_RETURN",0)) {
        freeTree(returnStmt);
        return NULL; // If KW_RETURN is not found, return NULL
    }

    // Parse EXP (nonterminal) - this can be either ARITH_EXP or BOOL_EXP
    TreeNode* exp = parseExp();
    if (!exp) {
        freeTree(returnStmt);
        return NULL; // If EXP parsing fails, return NULL
    }
    addChild(returnStmt, exp);  // Add EXP to the RETURN_STMT node

    // Match SEMICOLON token
    if (!match("SEMICOLON",0)) {
        freeTree(returnStmt);
        return NULL; // If SEMICOLON is not found, return NULL
    }

    // Return the successfully parsed RETURN_STMT node
    return returnStmt;
}

TreeNode* parseOutputStmt() {
    // Create the root node for OUTPUT_STMT
    TreeNode* outputStmt = createNode("OUTPUT_STMT");

    // Try to parse STD_OUTPUT
    TreeNode* stdOutput = parseStdOutput();
    if (stdOutput) {
        addChild(outputStmt, stdOutput); // If successful, add to the OUTPUT_STMT node
        return outputStmt;
    }

    // Try to parse VALUE_OUTPUT
    TreeNode* valueOutput = parseValueOutput();
    if (valueOutput) {
        addChild(outputStmt, valueOutput); // If successful, add to the OUTPUT_STMT node
        return outputStmt;
    }

    // Try to parse SEQUENCE_OUTPUT
    TreeNode* sequenceOutput = parseSequenceOutput();
    if (sequenceOutput) {
        addChild(outputStmt, sequenceOutput); // If successful, add to the OUTPUT_STMT node
        return outputStmt;
    }

    // If none of the above parsing attempts are successful, free the node and return NULL
    freeTree(outputStmt);
    return NULL;
}

TreeNode* parseStdOutput() {
    // Save the current token index to allow backtracking
    size_t savedIndex = currentTokenIndex;

    // Create the root node for STD_OUTPUT
    TreeNode* stdOutput = createNode("STD_OUTPUT");

    // Match KW_DISPLAY
    if (!match("KW_DISPLAY",0)) {
        freeTree(stdOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(stdOutput, createNode("KW_DISPLAY"));

    // Match LEFT_PAREN
    if (!match("LEFT_PAREN",0)) {
        freeTree(stdOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(stdOutput, createNode("LEFT_PAREN"));

    // Match STR_CONST
    if (!match("STR_CONST",0)) {
        freeTree(stdOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(stdOutput, createNode("STR_CONST"));

    // Match RIGHT_PAREN
    if (!match("RIGHT_PAREN",0)) {
        freeTree(stdOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(stdOutput, createNode("RIGHT_PAREN"));

    // Match SEMICOLON
    if (!match("SEMICOLON",0)) {
        freeTree(stdOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(stdOutput, createNode("SEMICOLON"));

    // Return the successfully parsed STD_OUTPUT tree
    return stdOutput;
}

TreeNode* parseValueOutput() {
    // Save the current token index to allow backtracking
    size_t savedIndex = currentTokenIndex;

    // Create the root node for VALUE_OUTPUT
    TreeNode* valueOutput = createNode("VALUE_OUTPUT");

    // Match KW_DISPLAY
    if (!match("KW_DISPLAY",0)) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, createNode("KW_DISPLAY"));

    // Match LEFT_PAREN
    if (!match("LEFT_PAREN",0)) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, createNode("LEFT_PAREN"));

    // Parse FORMAT_SPECIFIER (nonterminal)
    TreeNode* formatSpecifier = parseFormatSpecifier();
    if (!formatSpecifier) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, formatSpecifier);

    // Match COMMA
    if (!match("COMMA",0)) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, createNode("COMMA"));

    // Match IDENTIFIER
    if (!match("IDENTIFIER",0)) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, createNode("IDENTIFIER"));

    // Match RIGHT_PAREN
    if (!match("RIGHT_PAREN",0)) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, createNode("RIGHT_PAREN"));

    // Match SEMICOLON
    if (!match("SEMICOLON",0)) {
        freeTree(valueOutput);
        currentTokenIndex = savedIndex;
        return NULL;
    }
    addChild(valueOutput, createNode("SEMICOLON"));

    // Return the successfully parsed VALUE_OUTPUT tree
    return valueOutput;
}

TreeNode* parseFormatSpecifier() {
    // Create the root node for FORMAT_SPECIFIER
    TreeNode* formatSpecifier = createNode("FORMAT_SPECIFIER");

    // Match FORMAT_INT
    if (match("FORMAT_INT",0)) {
        addChild(formatSpecifier, createNode("FORMAT_INT"));
        return formatSpecifier;
    }

    // Match FORMAT_CHAR
    if (match("FORMAT_CHAR",0)) {
        addChild(formatSpecifier, createNode("FORMAT_CHAR"));
        return formatSpecifier;
    }

    // Match FORMAT_FLOAT
    if (match("FORMAT_FLOAT",0)) {
        addChild(formatSpecifier, createNode("FORMAT_FLOAT"));
        return formatSpecifier;
    }

    // Match FORMAT_STR
    if (match("FORMAT_STR",0)) {
        addChild(formatSpecifier, createNode("FORMAT_STR"));
        return formatSpecifier;
    }

    // If no matches, free the root node and return NULL
    freeTree(formatSpecifier);
    return NULL;
}

TreeNode* parseSequenceOutput() {
    // Create the root node for SEQUENCE_OUTPUT
    TreeNode* sequenceOutput = createNode("SEQUENCE_OUTPUT");

    // Match KW_DISPLAY
    if (!match("KW_DISPLAY",0)) {
        freeTree(sequenceOutput);
        return NULL;
    }
    addChild(sequenceOutput, createNode("KW_DISPLAY"));

    // Match LEFT_PAREN
    if (!match("LEFT_PAREN",0)) {
        freeTree(sequenceOutput);
        return NULL;
    }
    addChild(sequenceOutput, createNode("LEFT_PAREN"));

    // Match STR_WITH_FORMAT
    if (!match("STR_WITH_FORMAT",0)) {
        freeTree(sequenceOutput);
        return NULL;
    }
    addChild(sequenceOutput, createNode("STR_WITH_FORMAT"));

    // Match { COMMA OUTPUT_ELEM }
    while (match("COMMA",0)) {
        TreeNode* outputElem = parseOutputElem();
        if (outputElem == NULL) {
            freeTree(sequenceOutput);
            return NULL;
        }
        addChild(sequenceOutput, createNode("COMMA"));
        addChild(sequenceOutput, outputElem);
    }

    // Match RIGHT_PAREN
    if (!match("RIGHT_PAREN",0)) {
        freeTree(sequenceOutput);
        return NULL;
    }
    addChild(sequenceOutput, createNode("RIGHT_PAREN"));

    // Match SEMICOLON
    if (!match("SEMICOLON",0)) {
        freeTree(sequenceOutput);
        return NULL;
    }
    addChild(sequenceOutput, createNode("SEMICOLON"));

    return sequenceOutput;
}

TreeNode* parseOutputElem() {
    // Create the root node for OUTPUT_ELEM
    TreeNode* outputElem = createNode("OUTPUT_ELEM");

    // Match STR_CONST
    if (match("STR_CONST",0)) {
        addChild(outputElem, createNode("STR_CONST"));
        return outputElem;
    }

    // Match IDENTIFIER
    if (match("IDENTIFIER",0)) {
        addChild(outputElem, createNode("IDENTIFIER"));
        return outputElem;
    }

    // Parse ARITH_EXP
    TreeNode* arithExp = parseArithExp();
    if (arithExp) {
        addChild(outputElem, arithExp);
        return outputElem;
    }

    // If none of the cases match, free the tree and return NULL
    freeTree(outputElem);
    return NULL;
}

TreeNode* parseInputStmt() {
    // Create the root node for INPUT_STMT
    TreeNode* inputStmt = createNode("INPUT_STMT");

    // Match IDENTIFIER
    if (match("IDENTIFIER",0)) {
        addChild(inputStmt, createNode("IDENTIFIER"));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match '='
    if (match("ASSIGN_OP",0)) {
        addChild(inputStmt, createNode("="));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match KW_INPUT
    if (match("KW_INPUT",0)) {
        addChild(inputStmt, createNode("KW_INPUT"));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match LEFT_PAREN
    if (match("LEFT_PAREN",0)) {
        addChild(inputStmt, createNode("("));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match STR_CONST
    if (match("STR_CONST",0)) {
        addChild(inputStmt, createNode("STR_CONST"));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match COMMA
    if (match("COMMA",0)) {
        addChild(inputStmt, createNode(","));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Parse TYPE_SPEC
    TreeNode* typeSpec = parseTypeSpec();
    if (typeSpec) {
        addChild(inputStmt, typeSpec);
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match RIGHT_PAREN
    if (match("RIGHT_PAREN",0)) {
        addChild(inputStmt, createNode(")"));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    // Match SEMICOLON
    if (match("SEMICOLON",0)) {
        addChild(inputStmt, createNode(";"));
    } else {
        freeTree(inputStmt);
        return NULL;
    }

    return inputStmt;
}

void runParser(const char* symbol_table_file) {
    // Read the symbol table
    tokens = readSymbolTable(symbol_table_file, &token_count);
    if (!tokens) {
        fprintf(stderr, "Failed to read symbol table\n");
        return;
    }

    // Open parsed.txt for writing
    parsed_file = fopen("output/parsed.txt", "w");
    if (!parsed_file) {
        fprintf(stderr, "Failed to open output/parsed.txt for writing\n");
        // Clean up tokens before returning
        for (size_t i = 0; i < token_count; i++) {
            free(tokens[i].type);
            free(tokens[i].value);
        }
        free(tokens);
        return;
    }

    // Initialize parsing state
    currentTokenIndex = 0;
    nextNodeID = 0;

    // Parse the input starting from the top-level nonterminal
    TreeNode* parseTree = parseSimplicity();
    
    if (parseTree) {
        printf("Parsing successful!\n");
        fprintf(parsed_file, "Parsing successful!\n\n");

        // Write the parse tree in the original format to parsed.txt
        fprintf(parsed_file, "Parsed Tree:\n");
        writeParseTree(parsed_file, parseTree);

        // Write the CSV format
        FILE* csvFile = fopen("output/parse_tree.csv", "w");
        if (csvFile) {
            fprintf(csvFile, "NodeID,ParentID,Value\n");
            writeParseTree(csvFile, parseTree);
            fclose(csvFile);
        } else {
            fprintf(stderr, "Failed to open output/parse_tree.csv for writing\n");
        }

        // Write the parenthesized format
        FILE* txtFile = fopen("output/parse_tree_parenthesized.txt", "w");
        if (txtFile) {
            writeParseTreeParenthesized(txtFile, parseTree, 0);
            fclose(txtFile);
        } else {
            fprintf(stderr, "Failed to open output/parse_tree_parenthesized.txt for writing\n");
        }

        // Free the parse tree
        freeTree(parseTree);
    } else {
        // Report parsing failure
        printf("Parsing failed at token %zu: %s\n", 
               currentTokenIndex, 
               currentTokenIndex < token_count ? tokens[currentTokenIndex].type : "END");
        fprintf(parsed_file, "Parsing failed at token %zu: %s\n", 
                currentTokenIndex, 
                currentTokenIndex < token_count ? tokens[currentTokenIndex].type : "END");
    }

    // Clean up
    fclose(parsed_file);
    for (size_t i = 0; i < token_count; i++) {
        free(tokens[i].type);
        free(tokens[i].value);
    }
    free(tokens);
}

// New function to write parse tree in parenthesized format
void writeParseTreeParenthesized(FILE* file, TreeNode* node, int depth) {
    if (!node) return;
    
    // Add indentation for better readability
    for (int i = 0; i < depth; i++) {
        fprintf(file, "  ");
    }
    
    // Print the current node's value
    fprintf(file, "(%s", node->value);
    
    // If node has children, print them on new lines
    if (node->childCount > 0) {
        fprintf(file, "\n");
        for (size_t i = 0; i < node->childCount; i++) {
            writeParseTreeParenthesized(file, node->children[i], depth + 1);
        }
        // Close parenthesis on a new line with proper indentation
        for (int i = 0; i < depth; i++) {
            fprintf(file, "  ");
        }
        fprintf(file, ")\n");
    } else {
        // For leaf nodes, close parenthesis on the same line
        fprintf(file, ")\n");
    }
}