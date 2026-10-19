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
    NODE_EXPR_STMT,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_SWITCH,
    NODE_CASE,
    NODE_DEFAULT,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_BIN_OP,
    NODE_UNARY_OP,
    NODE_CALL,
    NODE_NEW,
    NODE_IDENTIFIER,
    NODE_LITERAL_INT,
    NODE_LITERAL_DOUBLE,
    NODE_LITERAL_FLOAT,
    NODE_LITERAL_LONG,
    NODE_LITERAL_CHAR,
    NODE_LITERAL_STRING,
    NODE_LITERAL_BOOL
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

        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_stmt;

        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_stmt;

        struct {
            ASTNode *body;
            ASTNode *condition;
        } do_while_stmt;

        struct {
            ASTNode *init;
            ASTNode *condition;
            ASTNode *increment;
            ASTNode *body;
        } for_stmt;

        struct {
            ASTNode *expr;
            ASTNode *cases;
        } switch_stmt;

        struct {
            ASTNode *value;
            ASTNode *statements;
        } case_stmt;

        struct {
            ASTNode *statements;
        } default_stmt;

        struct {
            ASTNode *expr;
        } return_stmt;

        struct {
            int op;
            ASTNode *left;
            ASTNode *right;
        } bin_op;

        struct {
            int op;
            int is_postfix;
            ASTNode *operand;
        } unary_op;

        struct {
            char *func_name;
            ASTNode *args;
        } call;

        struct {
            char *class_name;
            char *arg_name;
        } new_expr;

        struct {
            char *name;
        } identifier;

        struct {
            char *value;
        } literal;
    };
};

#endif
