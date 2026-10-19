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

ASTNode* ast_create_block(ASTNode *statements, int lineno) {
    ASTNode *node = ast_create_node(NODE_BLOCK, lineno);
    node->block.statements = statements;
    return node;
}

ASTNode* ast_create_assign(int op, char *var_name, ASTNode *expr, int lineno) {
    ASTNode *node = ast_create_node(NODE_ASSIGN, lineno);
    node->assign.op = op;
    node->assign.var_name = safe_strdup(var_name);
    node->assign.expr = expr;
    return node;
}

ASTNode* ast_create_expr_stmt(ASTNode *expr, int lineno) {
    ASTNode *node = ast_create_node(NODE_EXPR_STMT, lineno);
    node->expr_stmt.expr = expr;
    return node;
}

ASTNode* ast_create_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, int lineno) {
    ASTNode *node = ast_create_node(NODE_IF, lineno);
    node->if_stmt.condition = condition;
    node->if_stmt.then_branch = then_branch;
    node->if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_create_while(ASTNode *condition, ASTNode *body, int lineno) {
    ASTNode *node = ast_create_node(NODE_WHILE, lineno);
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;
    return node;
}

ASTNode* ast_create_do_while(ASTNode *body, ASTNode *condition, int lineno) {
    ASTNode *node = ast_create_node(NODE_DO_WHILE, lineno);
    node->do_while_stmt.body = body;
    node->do_while_stmt.condition = condition;
    return node;
}

ASTNode* ast_create_for(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body, int lineno) {
    ASTNode *node = ast_create_node(NODE_FOR, lineno);
    node->for_stmt.init = init;
    node->for_stmt.condition = condition;
    node->for_stmt.increment = increment;
    node->for_stmt.body = body;
    return node;
}

ASTNode* ast_create_switch(ASTNode *expr, ASTNode *cases, int lineno) {
    ASTNode *node = ast_create_node(NODE_SWITCH, lineno);
    node->switch_stmt.expr = expr;
    node->switch_stmt.cases = cases;
    return node;
}

ASTNode* ast_create_case(ASTNode *value, ASTNode *statements, int lineno) {
    ASTNode *node = ast_create_node(NODE_CASE, lineno);
    node->case_stmt.value = value;
    node->case_stmt.statements = statements;
    return node;
}

ASTNode* ast_create_default(ASTNode *statements, int lineno) {
    ASTNode *node = ast_create_node(NODE_DEFAULT, lineno);
    node->default_stmt.statements = statements;
    return node;
}

ASTNode* ast_create_return(ASTNode *expr, int lineno) {
    ASTNode *node = ast_create_node(NODE_RETURN, lineno);
    node->return_stmt.expr = expr;
    return node;
}

ASTNode* ast_create_break(int lineno) {
    return ast_create_node(NODE_BREAK, lineno);
}

ASTNode* ast_create_continue(int lineno) {
    return ast_create_node(NODE_CONTINUE, lineno);
}

ASTNode* ast_create_bin_op(int op, ASTNode *left, ASTNode *right, int lineno) {
    ASTNode *node = ast_create_node(NODE_BIN_OP, lineno);
    node->bin_op.op = op;
    node->bin_op.left = left;
    node->bin_op.right = right;
    return node;
}

ASTNode* ast_create_unary_op(int op, int is_postfix, ASTNode *operand, int lineno) {
    ASTNode *node = ast_create_node(NODE_UNARY_OP, lineno);
    node->unary_op.op = op;
    node->unary_op.is_postfix = is_postfix;
    node->unary_op.operand = operand;
    return node;
}

ASTNode* ast_create_call(char *func_name, ASTNode *args, int lineno) {
    ASTNode *node = ast_create_node(NODE_CALL, lineno);
    node->call.func_name = safe_strdup(func_name);
    node->call.args = args;
    return node;
}

ASTNode* ast_create_new(char *class_name, char *arg_name, int lineno) {
    ASTNode *node = ast_create_node(NODE_NEW, lineno);
    node->new_expr.class_name = safe_strdup(class_name);
    node->new_expr.arg_name = safe_strdup(arg_name);
    return node;
}

ASTNode* ast_create_identifier(char *name, int lineno) {
    ASTNode *node = ast_create_node(NODE_IDENTIFIER, lineno);
    node->identifier.name = safe_strdup(name);
    return node;
}

ASTNode* ast_create_literal(NodeType type, char *value, int lineno) {
    ASTNode *node = ast_create_node(type, lineno);
    node->literal.value = safe_strdup(value);
    return node;
}
