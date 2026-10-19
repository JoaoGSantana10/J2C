#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NODE_PROGRAM,
    NODE_CLASS
} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    int lineno;
    ASTNode *next;

    union {
        struct {
            char *name;
            ASTNode *body;
        } program;

        struct {
            char *name;
            ASTNode *members;
        } class_decl;
    };
};

#endif
