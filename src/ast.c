#include "ast.h"
#include <string.h>

static char* safe_strdup(const char *s) {
    if (!s) return NULL;
    char *copy = malloc(strlen(s) + 1);
    if (copy) {
        strcpy(copy, s);
    }
    return copy;
}

ASTNode* ast_create_node(NodeType type, int lineno) {
    ASTNode *node = (ASTNode*) calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Erro de alocacao de memoria para no da AST\n");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->lineno = lineno;
    node->next = NULL;
    return node;
}

ASTNode* ast_append(ASTNode *list, ASTNode *item) {
    if (!list) return item;
    if (!item) return list;

    ASTNode *curr = list;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = item;
    return list;
}

ASTNode* ast_create_program(char *name, ASTNode *body, int lineno) {
    ASTNode *node = ast_create_node(NODE_PROGRAM, lineno);
    node->program.name = safe_strdup(name);
    node->program.body = body;
    return node;
}

ASTNode* ast_create_class(int modifiers, char *name, ASTNode *members, int lineno) {
    ASTNode *node = ast_create_node(NODE_CLASS, lineno);
    node->class_decl.modifiers = modifiers;
    node->class_decl.name = safe_strdup(name);
    node->class_decl.members = members;
    return node;
}

ASTNode* ast_create_var_decl(int modifiers, char *type_name, char *name, ASTNode *init, int lineno) {
    ASTNode *node = ast_create_node(NODE_VAR_DECL, lineno);
    node->var_decl.modifiers = modifiers;
    node->var_decl.type_name = safe_strdup(type_name);
    node->var_decl.name = safe_strdup(name);
    node->var_decl.init = init;
    return node;
}

ASTNode* ast_create_func_decl(int modifiers, char *return_type, char *name, ASTNode *params, ASTNode *body, int lineno) {
    ASTNode *node = ast_create_node(NODE_FUNC_DECL, lineno);
    node->func_decl.modifiers = modifiers;
    node->func_decl.return_type = safe_strdup(return_type);
    node->func_decl.name = safe_strdup(name);
    node->func_decl.params = params;
    node->func_decl.body = body;
    return node;
}

ASTNode* ast_create_param(char *type_name, char *name, int is_array, int lineno) {
    ASTNode *node = ast_create_node(NODE_PARAM, lineno);
    node->param.type_name = safe_strdup(type_name);
    node->param.name = safe_strdup(name);
    node->param.is_array = is_array;
    return node;
}
