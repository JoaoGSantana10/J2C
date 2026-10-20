#include "ast.h"
#include <string.h>

char* ast_safe_strdup(const char *s) {
    if (!s) return NULL;
    char *copy = malloc(strlen(s) + 1);
    if (!copy) {
        fprintf(stderr, "Falha ao tentar alocar uma String\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, s);
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

ASTNode* ast_create_program(ASTNode *body, int lineno) {
    ASTNode *node = ast_create_node(NODE_PROGRAM, lineno);
    node->program.body = body;
    return node;
}

ASTNode* ast_create_class(int modifiers, char *name, ASTNode *members, int lineno) {
    ASTNode *node = ast_create_node(NODE_CLASS, lineno);
    node->class_decl.modifiers = modifiers;
    node->class_decl.name = name;
    node->class_decl.members = members;
    return node;
}

ASTNode* ast_create_var_decl(int modifiers, char *type_name, char *name, ASTNode *init, int lineno) {
    ASTNode *node = ast_create_node(NODE_VAR_DECL, lineno);
    node->var_decl.modifiers = modifiers;
    node->var_decl.type_name = type_name;
    node->var_decl.name = name;
    node->var_decl.init = init;
    return node;
}

ASTNode* ast_create_func_decl(int modifiers, char *return_type, char *name, ASTNode *params, ASTNode *body, int lineno) {
    ASTNode *node = ast_create_node(NODE_FUNC_DECL, lineno);
    node->func_decl.modifiers = modifiers;
    node->func_decl.return_type = return_type;
    node->func_decl.name = name;
    node->func_decl.params = params;
    node->func_decl.body = body;
    return node;
}

ASTNode* ast_create_param(char *type_name, char *name, int is_array, int lineno) {
    ASTNode *node = ast_create_node(NODE_PARAM, lineno);
    node->param.type_name = type_name;
    node->param.name = name;
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
    node->assign.var_name = var_name;
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
    node->call.func_name = func_name;
    node->call.args = args;
    return node;
}

ASTNode* ast_create_new(char *class_name, char *arg_name, int lineno) {
    ASTNode *node = ast_create_node(NODE_NEW, lineno);
    node->new_expr.class_name = class_name;
    node->new_expr.arg_name = arg_name;
    return node;
}

ASTNode* ast_create_identifier(char *name, int lineno) {
    ASTNode *node = ast_create_node(NODE_IDENTIFIER, lineno);
    node->identifier.name = name;
    return node;
}

ASTNode* ast_create_literal(NodeType type, char *value, int lineno) {
    ASTNode *node = ast_create_node(type, lineno);
    node->literal.value = value;
    return node;
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print(const ASTNode *node, int indent) {
    while (node) {
        print_indent(indent);
        switch (node->type) {
            case NODE_PROGRAM:
                printf("Program (linha %d)\n", node->lineno);
                ast_print(node->program.body, indent + 1);
                break;
            case NODE_CLASS:
                printf("Class: %s (linha %d)\n", node->class_decl.name, node->lineno);
                ast_print(node->class_decl.members, indent + 1);
                break;
            case NODE_VAR_DECL:
                printf("VarDecl: %s %s (linha %d)\n", node->var_decl.type_name, node->var_decl.name, node->lineno);
                if (node->var_decl.init) {
                    print_indent(indent + 1);
                    printf("Init:\n");
                    ast_print(node->var_decl.init, indent + 2);
                }
                break;
            case NODE_FUNC_DECL:
                printf("FuncDecl: %s %s (linha %d)\n", node->func_decl.return_type, node->func_decl.name, node->lineno);
                if (node->func_decl.params) {
                    print_indent(indent + 1);
                    printf("Params:\n");
                    ast_print(node->func_decl.params, indent + 2);
                }
                if (node->func_decl.body) {
                    ast_print(node->func_decl.body, indent + 1);
                }
                break;
            case NODE_PARAM:
                printf("Param: %s %s%s\n", node->param.type_name, node->param.name, node->param.is_array ? "[]" : "");
                break;
            case NODE_BLOCK:
                printf("Block (linha %d)\n", node->lineno);
                ast_print(node->block.statements, indent + 1);
                break;
            case NODE_ASSIGN:
                printf("Assign: %s op=%d (linha %d)\n", node->assign.var_name, node->assign.op, node->lineno);
                ast_print(node->assign.expr, indent + 1);
                break;
            case NODE_EXPR_STMT:
                printf("ExprStmt (linha %d)\n", node->lineno);
                ast_print(node->expr_stmt.expr, indent + 1);
                break;
            case NODE_IF:
                printf("If (linha %d)\n", node->lineno);
                print_indent(indent + 1);
                printf("Condition:\n");
                ast_print(node->if_stmt.condition, indent + 2);
                print_indent(indent + 1);
                printf("Then:\n");
                ast_print(node->if_stmt.then_branch, indent + 2);
                if (node->if_stmt.else_branch) {
                    print_indent(indent + 1);
                    printf("Else:\n");
                    ast_print(node->if_stmt.else_branch, indent + 2);
                }
                break;
            case NODE_WHILE:
                printf("While (linha %d)\n", node->lineno);
                print_indent(indent + 1);
                printf("Condition:\n");
                ast_print(node->while_stmt.condition, indent + 2);
                print_indent(indent + 1);
                printf("Body:\n");
                ast_print(node->while_stmt.body, indent + 2);
                break;
            case NODE_DO_WHILE:
                printf("DoWhile (linha %d)\n", node->lineno);
                print_indent(indent + 1);
                printf("Body:\n");
                ast_print(node->do_while_stmt.body, indent + 2);
                print_indent(indent + 1);
                printf("Condition:\n");
                ast_print(node->do_while_stmt.condition, indent + 2);
                break;
            case NODE_FOR:
                printf("For (linha %d)\n", node->lineno);
                if (node->for_stmt.init) {
                    print_indent(indent + 1);
                    printf("Init:\n");
                    ast_print(node->for_stmt.init, indent + 2);
                }
                if (node->for_stmt.condition) {
                    print_indent(indent + 1);
                    printf("Condition:\n");
                    ast_print(node->for_stmt.condition, indent + 2);
                }
                if (node->for_stmt.increment) {
                    print_indent(indent + 1);
                    printf("Increment:\n");
                    ast_print(node->for_stmt.increment, indent + 2);
                }
                print_indent(indent + 1);
                printf("Body:\n");
                ast_print(node->for_stmt.body, indent + 2);
                break;
            case NODE_SWITCH:
                printf("Switch (linha %d)\n", node->lineno);
                print_indent(indent + 1);
                printf("Expr:\n");
                ast_print(node->switch_stmt.expr, indent + 2);
                ast_print(node->switch_stmt.cases, indent + 1);
                break;
            case NODE_CASE:
                printf("Case (linha %d)\n", node->lineno);    
                print_indent(indent + 1);
                printf("Value:\n");
                ast_print(node->case_stmt.value, indent + 2);
                ast_print(node->case_stmt.statements, indent + 1);
                break;
            case NODE_DEFAULT:
                printf("Default (linha %d)\n", node->lineno);
                ast_print(node->default_stmt.statements, indent + 1);
                break;
            case NODE_RETURN:
                printf("Return (linha %d)\n", node->lineno);
                if (node->return_stmt.expr) {
                    ast_print(node->return_stmt.expr, indent + 1);
                }
                break;
            case NODE_BREAK:
                printf("Break (linha %d)\n", node->lineno);
                break;
            case NODE_CONTINUE:
                printf("Continue (linha %d)\n", node->lineno);
                break;
            case NODE_BIN_OP:
                printf("BinOp: op=%d (linha %d)\n", node->bin_op.op, node->lineno);
                ast_print(node->bin_op.left, indent + 1);
                ast_print(node->bin_op.right, indent + 1);
                break;
            case NODE_UNARY_OP:
                printf("UnaryOp: op=%d postfix=%d (linha %d)\n", node->unary_op.op, node->unary_op.is_postfix, node->lineno);
                ast_print(node->unary_op.operand, indent + 1);
                break;
            case NODE_CALL:
                printf("Call: %s (linha %d)\n", node->call.func_name, node->lineno);
                if (node->call.args) {
                    ast_print(node->call.args, indent + 1);
                }
                break;
            case NODE_NEW:
                printf("New: %s (linha %d)\n", node->new_expr.class_name, node->lineno);
                break;
            case NODE_IDENTIFIER:
                printf("Id: %s (linha %d)\n", node->identifier.name, node->lineno);
                break;
            case NODE_LITERAL_INT:
            case NODE_LITERAL_DOUBLE:
            case NODE_LITERAL_FLOAT:
            case NODE_LITERAL_LONG:
            case NODE_LITERAL_CHAR:
            case NODE_LITERAL_STRING:
            case NODE_LITERAL_BOOL:
                printf("Literal: %s (linha %d)\n", node->literal.value, node->lineno);
                break;
        }
        node = node->next;
    }
}

void ast_free(ASTNode *node) {
    while (node) {
        ASTNode *next = node->next;

        switch (node->type) {
            case NODE_PROGRAM:
                ast_free(node->program.body);
                break;
            case NODE_CLASS:
                free(node->class_decl.name);
                ast_free(node->class_decl.members);
                break;
            case NODE_VAR_DECL:
                free(node->var_decl.type_name);
                free(node->var_decl.name);
                ast_free(node->var_decl.init);
                break;
            case NODE_FUNC_DECL:
                free(node->func_decl.return_type);
                free(node->func_decl.name);
                ast_free(node->func_decl.params);
                ast_free(node->func_decl.body);
                break;
            case NODE_PARAM:
                free(node->param.type_name);
                free(node->param.name);
                break;
            case NODE_BLOCK:
                ast_free(node->block.statements);
                break;
            case NODE_ASSIGN:
                free(node->assign.var_name);
                ast_free(node->assign.expr);
                break;
            case NODE_EXPR_STMT:
                ast_free(node->expr_stmt.expr);
                break;
            case NODE_IF:
                ast_free(node->if_stmt.condition);
                ast_free(node->if_stmt.then_branch);
                ast_free(node->if_stmt.else_branch);
                break;
            case NODE_WHILE:
                ast_free(node->while_stmt.condition);
                ast_free(node->while_stmt.body);
                break;
            case NODE_DO_WHILE:
                ast_free(node->do_while_stmt.body);
                ast_free(node->do_while_stmt.condition);
                break;
            case NODE_FOR:
                ast_free(node->for_stmt.init);
                ast_free(node->for_stmt.condition);
                ast_free(node->for_stmt.increment);
                ast_free(node->for_stmt.body);
                break;
            case NODE_SWITCH:
                ast_free(node->switch_stmt.expr);
                ast_free(node->switch_stmt.cases);
                break;
            case NODE_CASE:
                ast_free(node->case_stmt.value);
                ast_free(node->case_stmt.statements);
                break;
            case NODE_DEFAULT:
                ast_free(node->default_stmt.statements);
                break;
            case NODE_RETURN:
                ast_free(node->return_stmt.expr);
                break;
            case NODE_BREAK:
            case NODE_CONTINUE:
                break;
            case NODE_BIN_OP:
                ast_free(node->bin_op.left);
                ast_free(node->bin_op.right);
                break;
            case NODE_UNARY_OP:
                ast_free(node->unary_op.operand);
                break;
            case NODE_CALL:
                free(node->call.func_name);
                ast_free(node->call.args);
                break;
            case NODE_NEW:
                free(node->new_expr.class_name);
                free(node->new_expr.arg_name);
                break;
            case NODE_IDENTIFIER:
                free(node->identifier.name);
                break;
            case NODE_LITERAL_INT:
            case NODE_LITERAL_DOUBLE:
            case NODE_LITERAL_FLOAT:
            case NODE_LITERAL_LONG:
            case NODE_LITERAL_CHAR:
            case NODE_LITERAL_STRING:
            case NODE_LITERAL_BOOL:
                free(node->literal.value);
                break;
        }

        free(node);
        node = next;
    }
}
