#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int line_num;
} TokenInfo;

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

TokenInfo* readSymbolTable(const char* filename, size_t* token_count);
int match(const char* expectedType, int isOptional);
TreeNode* createNode(const char* value);
void addChild(TreeNode* parent, TreeNode* child);
void freeTree(TreeNode* node);
void writeParsingState();
void writeParseTree(FILE* file, TreeNode* node, int depth);

int runParser(const char* tokenFile);

#endif // PARSER_H