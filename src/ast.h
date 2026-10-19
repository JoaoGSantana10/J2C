#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NODE_PROGRAM,
    NODE_CLASS,
    NODE_VAR_DECL,
    NODE_FUNC_DECL,
    NODE_PARAM,
    NODE_BLOCK,
    NODE_ASSIGN,
    NODE_EXPR_STMT
} NodeType;

typedef enum {
    MOD_NONE      = 0,
    MOD_PUBLIC    = 1 << 0,
    MOD_PRIVATE   = 1 << 1,
    MOD_PROTECTED = 1 << 2,
    MOD_STATIC    = 1 << 3
} ModifierFlags;

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
            int modifiers;
            char *name;
            ASTNode *members;
        } class_decl;

        struct {
            int modifiers;
            char *type_name;
            char *name;
            ASTNode *init;
        } var_decl;

        struct {
            int modifiers;
            char *return_type;
            char *name;
            ASTNode *params;
            ASTNode *body;
        } func_decl;

        struct {
            char *type_name;
            char *name;
            int is_array;
        } param;

        struct {
            ASTNode *statements;
        } block;

        struct {
            int op;
            char *var_name;
            ASTNode *expr;
        } assign;

        struct {
            ASTNode *expr;
        } expr_stmt;
    };
};

#endif
