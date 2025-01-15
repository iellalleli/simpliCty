#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the maximum number of children for a parse tree node
#define MAX_CHILDREN 10

// Define the TreeNode structure
typedef struct TreeNode {
    int nodeID;               // Unique ID for the node
    int parentID;             // Parent node ID
    char value[256];          // Value or label of the node
    int childCount;           // Number of children
    struct TreeNode* children[MAX_CHILDREN]; // Array of child nodes
} TreeNode;

// Function declarations

/**
 * Create a new parse tree node.
 * @param id The unique ID of the node.
 * @param parentID The ID of the parent node.
 * @param value The value or label of the node.
 * @return A pointer to the newly created TreeNode.
 */
TreeNode* createNode(int id, int parentID, const char* value);

/**
 * Add a child node to a parent node.
 * @param parent The parent TreeNode.
 * @param child The child TreeNode to add.
 */
void addChild(TreeNode* parent, TreeNode* child);

/**
 * Save the parse tree to a CSV file.
 * @param root The root of the parse tree.
 * @param filename The name of the CSV file to save to.
 */
void saveParseTreeToCSV(TreeNode* root, const char* filename);

/**
 * Log parsing output messages to a text file.
 * @param message The message to log.
 * @param filename The name of the text file to log to.
 */
void logParsedOutput(const char* message, const char* filename);

/**
 * Advance the current token in the lexer.
 */
void advanceToken();

/**
 * Match and consume a token of the expected type.
 * @param expectedType The expected token type.
 * @return 1 if the token matches, 0 otherwise.
 */
int matchToken(const char* expectedType);

/**
 * Parse an expression.
 * @param parent The parent TreeNode for the expression.
 * @return A pointer to the TreeNode representing the parsed expression.
 */
TreeNode* parseExpression(TreeNode* parent);

/**
 * Parse an assignment statement.
 * @param parent The parent TreeNode for the assignment.
 * @return A pointer to the TreeNode representing the parsed assignment.
 */
TreeNode* parseAssignment(TreeNode* parent);

/**
 * Run the parser on a token file.
 * @param tokenFile The file containing tokens to parse.
 * @return 1 if parsing succeeds, 0 otherwise.
 */
int runParser(const char* tokenFile);

#endif // PARSER_H
