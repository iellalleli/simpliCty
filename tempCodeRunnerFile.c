#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexers.h"

typedef struct TreeNode {
    int id;
    int parentID;
    char *value;
    struct TreeNode **children;
    size_t childCount;
} TreeNode;

static size_t nextNodeID = 0;

// Function prototypes
TreeNode* createNode(int id, int parentID, const char *value);
void addChild(TreeNode *parent, TreeNode *child);
void freeTree(TreeNode *node);
void parseProgram(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile);
void parseStatement(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile);
void parseAssignment(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile);
void parseExpression(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile);
void writeTreeToFile(TreeNode *node, FILE *debugFile, FILE *csvFile, int level);
void writeTokenDebug(FILE *debugFile, Token *token);

// TreeNode functions
TreeNode* createNode(int id, int parentID, const char *value) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->id = id;
    node->parentID = parentID;
    node->value = strdup(value);
    node->children = NULL;
    node->childCount = 0;
    return node;
}

void addChild(TreeNode *parent, TreeNode *child) {
    parent->children = (TreeNode **)realloc(parent->children, sizeof(TreeNode *) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

void freeTree(TreeNode *node) {
    if (!node) return;
    for (size_t i = 0; i < node->childCount; ++i) {
        freeTree(node->children[i]);
    }
    free(node->value);
    free(node->children);
    free(node);
}

// Helper function to write token debug information
void writeTokenDebug(FILE *debugFile, Token *token) {
    fprintf(debugFile, "TYPE: %s | TOKEN: %s | LEXEME: %s\n", token->type, token->value, token->value);
}

// Parsing functions
void parseProgram(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile) {
    while (*index < token_count) {
        TreeNode *statementNode = createNode(nextNodeID++, node->id, "Statement");
        parseStatement(statementNode, tokens, index, token_count, debugFile);
        addChild(node, statementNode);
    }
}

void parseStatement(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile) {
    Token *current = tokens[*index];
    if (current->type == IDENTIFIER) {
        writeTokenDebug(debugFile, current);
        parseAssignment(node, tokens, index, token_count, debugFile);
    } else if (current->type == KW_MAIN) {
        writeTokenDebug(debugFile, current);
        (*index)++;
        if (*index < token_count && tokens[*index]->type == LEFT_PAREN) {
            writeTokenDebug(debugFile, tokens[*index]);
            (*index)++;
        }
        if (*index < token_count && tokens[*index]->type == RIGHT_PAREN) {
            writeTokenDebug(debugFile, tokens[*index]);
            (*index)++;
        }
        if (*index < token_count && tokens[*index]->type == LEFT_CURLY) {
            writeTokenDebug(debugFile, tokens[*index]);
            (*index)++;
        }

        while (*index < token_count && tokens[*index]->type != RIGHT_CURLY) {
            TreeNode *child = createNode(nextNodeID++, node->id, "Statement");
            parseStatement(child, tokens, index, token_count, debugFile);
            addChild(node, child);
        }
        if (*index < token_count && tokens[*index]->type == RIGHT_CURLY) {
            writeTokenDebug(debugFile, tokens[*index]);
            (*index)++;
        }
    }
}

void parseAssignment(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile) {
    if (*index < token_count && tokens[*index]->type == IDENTIFIER) {
        writeTokenDebug(debugFile, tokens[*index]);
        TreeNode *varNode = createNode(nextNodeID++, node->id, tokens[*index]->value);
        addChild(node, varNode);
        (*index)++;

        if (*index < token_count && tokens[*index]->type == ASSIGN_OP) {
            writeTokenDebug(debugFile, tokens[*index]);
            TreeNode *assignNode = createNode(nextNodeID++, node->id, "=");
            addChild(node, assignNode);
            (*index)++;
            parseExpression(assignNode, tokens, index, token_count, debugFile);
        }

        if (*index < token_count && tokens[*index]->type == SEMICOLON) {
            writeTokenDebug(debugFile, tokens[*index]);
            (*index)++;
        }
    }
}

void parseExpression(TreeNode *node, Token **tokens, size_t *index, size_t token_count, FILE *debugFile) {
    if (*index < token_count && 
       (tokens[*index]->type == NUM_CONST || tokens[*index]->type == IDENTIFIER)) {
        writeTokenDebug(debugFile, tokens[*index]);
        TreeNode *exprNode = createNode(nextNodeID++, node->id, tokens[*index]->value);
        addChild(node, exprNode);
        (*index)++;
    }
}

// Helper function to write the parse tree to files
void writeTreeToFile(TreeNode *node, FILE *debugFile, FILE *csvFile, int level) {
    // Write to debug file
    for (int i = 0; i < level; i++) fprintf(debugFile, "  ");
    fprintf(debugFile, "Node(ID=%d, Value='%s', ParentID=%d)\n", node->id, node->value, node->parentID);

    // Write to CSV file
    fprintf(csvFile, "%d,%d,%s\n", node->id, node->parentID, node->value);

    // Recursively write children
    for (size_t i = 0; i < node->childCount; i++) {
        writeTreeToFile(node->children[i], debugFile, csvFile, level + 1);
    }
}

// Updated runParser function
void runParser(Token **tokens, size_t token_count) {
    TreeNode *root = createNode(nextNodeID++, -1, "Program");
    size_t index = 0;

    // Open debug file early
    FILE *debugFile = fopen("output/parsed.txt", "w");
    if (!debugFile) {
        fprintf(stderr, "Error: Unable to create debug file.\n");
        freeTree(root);
        return;
    }

    parseProgram(root, tokens, &index, token_count, debugFile);

    printf("Parsing completed. Syntax tree constructed.\n");

    // Open CSV file
    FILE *csvFile = fopen("output/parse_tree.csv", "w");
    if (!csvFile) {
        fprintf(stderr, "Error: Unable to create CSV file.\n");
        freeTree(root);
        fclose(debugFile);
        return;
    }

    // Write headers for CSV file
    fprintf(csvFile, "NodeID,ParentID,Value\n");

    // Write the tree to both files
    writeTreeToFile(root, debugFile, csvFile, 0);

    fclose(debugFile);
    fclose(csvFile);

    // Free the tree after parsing
    freeTree(root);
}