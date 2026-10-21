#ifndef AST_H
#define AST_H

/* 
 * Regra de posse das strings:
 * - Toda string passada a um construtor ast_create_* deve ser alocada dinamicamente com ast_safe_strdup()
 *   (dica: qual função usar para isso?).
 * - Depois de passada, a string pertence ao nó e é liberada pelo ast_free.
 *   Quem chamou não deve free na string nem usá-la após depois.
 * - Nunca passar a mesma string para dois construtores diferentes.
*/

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

typedef enum{
    AST_OP_ADD /* '+' */,
    AST_OP_SUB /* '-' */,
    AST_OP_MUL /* '*' */,
    AST_OP_DIV /* '/' */,
    AST_OP_MOD /* '%' */,
    AST_OP_EQ  /* '==' */,
    AST_OP_NE  /* '!=' */,
    AST_OP_LT  /* '<' */,
    AST_OP_GT  /* '>' */,
    AST_OP_LE  /* '<=' */,
    AST_OP_GE  /* '>=' */,
    AST_OP_AND /* '&&' */,
    AST_OP_OR  /* '||' */,
} BinaryOpType;

typedef enum{
    AST_UN_INC /* '++' */,
    AST_UN_DEC /* '--' */,
    AST_UN_NEG /* '-' */,
    AST_UN_NOT /* '!' */,
} UnaryOpType;

typedef enum{
    AST_ASG_ASSIGN /* '=' */,
    AST_ASG_PLUS_ASSIGN /* '+=' */,
    AST_ASG_MINUS_ASSIGN /* '-=' */,
} AssignOpType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    int lineno;
    ASTNode *next;

    union {
        struct {
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
            AssignOpType op;
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
            BinaryOpType op;
            ASTNode *left;
            ASTNode *right;
        } bin_op;

        struct {
            UnaryOpType op;
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

/* Funcoes base e listas */
ASTNode* ast_create_node(NodeType type, int lineno);
ASTNode* ast_append(ASTNode *list, ASTNode *item);
char* ast_safe_strdup(const char *s);

/* Construtores de programa e declaracoes */
ASTNode* ast_create_program(ASTNode *body, int lineno);
ASTNode* ast_create_class(int modifiers, char *name, ASTNode *members, int lineno);
ASTNode* ast_create_var_decl(int modifiers, char *type_name, char *name, ASTNode *init, int lineno);
ASTNode* ast_create_func_decl(int modifiers, char *return_type, char *name, ASTNode *params, ASTNode *body, int lineno);
ASTNode* ast_create_param(char *type_name, char *name, int is_array, int lineno);

/* Construtores de comandos */
ASTNode* ast_create_block(ASTNode *statements, int lineno);
ASTNode* ast_create_assign(AssignOpType op, char *var_name, ASTNode *expr, int lineno);
ASTNode* ast_create_expr_stmt(ASTNode *expr, int lineno);
ASTNode* ast_create_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, int lineno);
ASTNode* ast_create_while(ASTNode *condition, ASTNode *body, int lineno);
ASTNode* ast_create_do_while(ASTNode *body, ASTNode *condition, int lineno);
ASTNode* ast_create_for(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body, int lineno);
ASTNode* ast_create_switch(ASTNode *expr, ASTNode *cases, int lineno);
ASTNode* ast_create_case(ASTNode *value, ASTNode *statements, int lineno);
ASTNode* ast_create_default(ASTNode *statements, int lineno);
ASTNode* ast_create_return(ASTNode *expr, int lineno);
ASTNode* ast_create_break(int lineno);
ASTNode* ast_create_continue(int lineno);

/* Construtores de expressoes */
ASTNode* ast_create_bin_op(BinaryOpType op, ASTNode *left, ASTNode *right, int lineno);
ASTNode* ast_create_unary_op(UnaryOpType op, int is_postfix, ASTNode *operand, int lineno);
ASTNode* ast_create_call(char *func_name, ASTNode *args, int lineno);
ASTNode* ast_create_new(char *class_name, char *arg_name, int lineno);
ASTNode* ast_create_identifier(char *name, int lineno);
ASTNode* ast_create_literal(NodeType type, char *value, int lineno);

/* Impressao e liberacao de memoria */
void ast_print(const ASTNode *node, int indent);
void ast_free(ASTNode *node);

#endif
