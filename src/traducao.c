#include "traducao.h"
#include <string.h>

static void emit_indent(FILE *out, int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(out, "    ");
    }
}

static const char* mapear_tipo(const char *tipo_java) {
    if (!tipo_java) return "void";
    if (strcmp(tipo_java, "boolean") == 0) return "bool";
    if (strcmp(tipo_java, "String") == 0) return "char*";
    return tipo_java;
}

void traduzir_expressao(FILE *out, const ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_IDENTIFIER:
            fprintf(out, "%s", node->identifier.name);
            break;

        case NODE_LITERAL_INT:
        case NODE_LITERAL_DOUBLE:
        case NODE_LITERAL_FLOAT:
        case NODE_LITERAL_LONG:
        case NODE_LITERAL_CHAR:
        case NODE_LITERAL_STRING:
            fprintf(out, "%s", node->literal.value);
            break;

        case NODE_LITERAL_BOOL:
            fprintf(out, "%s", (strcmp(node->literal.value, "true") == 0) ? "true" : "false");
            break;

        case NODE_BIN_OP: {
            const char *op_str = "+";
            switch (node->bin_op.op) {
                case AST_OP_ADD: op_str = "+"; break;
                case AST_OP_SUB: op_str = "-"; break;
                case AST_OP_MUL: op_str = "*"; break;
                case AST_OP_DIV: op_str = "/"; break;
                case AST_OP_MOD: op_str = "%"; break;
                case AST_OP_EQ:  op_str = "=="; break;
                case AST_OP_NE:  op_str = "!="; break;
                case AST_OP_LT:  op_str = "<"; break;
                case AST_OP_GT:  op_str = ">"; break;
                case AST_OP_LE:  op_str = "<="; break;
                case AST_OP_GE:  op_str = ">="; break;
                case AST_OP_AND: op_str = "&&"; break;
                case AST_OP_OR:  op_str = "||"; break;
            }
            fprintf(out, "(");
            traduzir_expressao(out, node->bin_op.left);
            fprintf(out, " %s ", op_str);
            traduzir_expressao(out, node->bin_op.right);
            fprintf(out, ")");
            break;
        }

        case NODE_UNARY_OP: {
            const char *op_str = "!";
            switch (node->unary_op.op) {
                case AST_UN_INC: op_str = "++"; break;
                case AST_UN_DEC: op_str = "--"; break;
                case AST_UN_NEG: op_str = "-"; break;
                case AST_UN_NOT: op_str = "!"; break;
            }
            if (node->unary_op.is_postfix) {
                traduzir_expressao(out, node->unary_op.operand);
                fprintf(out, "%s", op_str);
            } else {
                fprintf(out, "%s", op_str);
                traduzir_expressao(out, node->unary_op.operand);
            }
            break;
        }

        case NODE_CALL: {
            if (strcmp(node->call.func_name, "System.out.println") == 0 ||
                strcmp(node->call.func_name, "System.out.print") == 0) {
                int newline = (strcmp(node->call.func_name, "System.out.println") == 0);
                if (node->call.args && node->call.args->type == NODE_LITERAL_STRING) {
                    fprintf(out, "printf(\"%%s%s\", ", newline ? "\\n" : "");
                    traduzir_expressao(out, node->call.args);
                    fprintf(out, ")");
                } else if (node->call.args) {
                    fprintf(out, "printf(\"%%d%s\", ", newline ? "\\n" : "");
                    traduzir_expressao(out, node->call.args);
                    fprintf(out, ")");
                } else {
                    fprintf(out, "printf(\"%s\")", newline ? "\\n" : "");
                }
            } else {
                fprintf(out, "%s(", node->call.func_name);
                for (const ASTNode *arg = node->call.args; arg; arg = arg->next) {
                    traduzir_expressao(out, arg);
                    if (arg->next) fprintf(out, ", ");
                }
                fprintf(out, ")");
            }
            break;
        }

        default:
            break;
    }
}

static void traduzir_corpo_comando(FILE *out, const ASTNode *body, int indent) {
    if (!body) {
        fprintf(out, " {}\n");
        return;
    }
    if (body->type == NODE_BLOCK) {
        fprintf(out, "{\n");
        for (const ASTNode *s = body->block.statements; s; s = s->next) {
            traduzir_comando(out, s, indent + 1);
        }
        emit_indent(out, indent);
        fprintf(out, "}");
    } else {
        fprintf(out, "\n");
        traduzir_comando(out, body, indent + 1);
    }
}

static void traduzir_if(FILE *out, const ASTNode *node, int indent) {
    fprintf(out, "if (");
    traduzir_expressao(out, node->if_stmt.condition);
    fprintf(out, ") ");
    traduzir_corpo_comando(out, node->if_stmt.then_branch, indent);

    if (node->if_stmt.else_branch) {
        if (node->if_stmt.then_branch && node->if_stmt.then_branch->type == NODE_BLOCK) {
            fprintf(out, " else ");
        } else {
            emit_indent(out, indent);
            fprintf(out, "else ");
        }

        if (node->if_stmt.else_branch->type == NODE_IF) {
            traduzir_if(out, node->if_stmt.else_branch, indent);
        } else {
            traduzir_corpo_comando(out, node->if_stmt.else_branch, indent);
            if (node->if_stmt.else_branch->type == NODE_BLOCK) {
                fprintf(out, "\n");
            }
        }
    } else {
        if (node->if_stmt.then_branch && node->if_stmt.then_branch->type == NODE_BLOCK) {
            fprintf(out, "\n");
        }
    }
}

static void traduzir_atribuicao_sem_terminador(FILE *out, const ASTNode *node) {
    if (!node || node->type != NODE_ASSIGN) return;
    const char *op_str = "=";
    if (node->assign.op == AST_ASG_PLUS_ASSIGN) {
        op_str = "+=";
    } else if (node->assign.op == AST_ASG_MINUS_ASSIGN) {
        op_str = "-=";
    }
    fprintf(out, "%s %s ", node->assign.var_name, op_str);
    traduzir_expressao(out, node->assign.expr);
}

static void traduzir_for(FILE *out, const ASTNode *node, int indent) {
    fprintf(out, "for (");
    if (node->for_stmt.init) {
        if (node->for_stmt.init->type == NODE_VAR_DECL) {
            fprintf(out, "%s %s", mapear_tipo(node->for_stmt.init->var_decl.type_name),
                    node->for_stmt.init->var_decl.name);
            if (node->for_stmt.init->var_decl.init) {
                fprintf(out, " = ");
                traduzir_expressao(out, node->for_stmt.init->var_decl.init);
            }
        } else if (node->for_stmt.init->type == NODE_ASSIGN) {
            traduzir_atribuicao_sem_terminador(out, node->for_stmt.init);
        }
    }
    fprintf(out, ";");
    if (node->for_stmt.condition) {
        fprintf(out, " ");
        traduzir_expressao(out, node->for_stmt.condition);
    }
    fprintf(out, ";");
    if (node->for_stmt.increment) {
        fprintf(out, " ");
        if (node->for_stmt.increment->type == NODE_ASSIGN) {
            traduzir_atribuicao_sem_terminador(out, node->for_stmt.increment);
        } else {
            traduzir_expressao(out, node->for_stmt.increment);
        }
    }
    fprintf(out, ") ");
    traduzir_corpo_comando(out, node->for_stmt.body, indent);
    if (node->for_stmt.body && node->for_stmt.body->type == NODE_BLOCK) {
        fprintf(out, "\n");
    }
}

void traduzir_comando(FILE *out, const ASTNode *node, int indent) {
    if (!node) return;

    switch (node->type) {
        case NODE_BLOCK: {
            emit_indent(out, indent);
            fprintf(out, "{\n");
            for (const ASTNode *s = node->block.statements; s; s = s->next) {
                traduzir_comando(out, s, indent + 1);
            }
            emit_indent(out, indent);
            fprintf(out, "}\n");
            break;
        }

        case NODE_VAR_DECL:
            emit_indent(out, indent);
            fprintf(out, "%s %s", mapear_tipo(node->var_decl.type_name), node->var_decl.name);
            if (node->var_decl.init) {
                fprintf(out, " = ");
                traduzir_expressao(out, node->var_decl.init);
            }
            fprintf(out, ";\n");
            break;

        case NODE_ASSIGN:
            emit_indent(out, indent);
            traduzir_atribuicao_sem_terminador(out, node);
            fprintf(out, ";\n");
            break;

        case NODE_EXPR_STMT:
            emit_indent(out, indent);
            traduzir_expressao(out, node->expr_stmt.expr);
            fprintf(out, ";\n");
            break;

        case NODE_BREAK:
            emit_indent(out, indent);
            fprintf(out, "break;\n");
            break;

        case NODE_CONTINUE:
            emit_indent(out, indent);
            fprintf(out, "continue;\n");
            break;

        case NODE_RETURN:
            emit_indent(out, indent);
            if (node->return_stmt.expr) {
                fprintf(out, "return ");
                traduzir_expressao(out, node->return_stmt.expr);
                fprintf(out, ";\n");
            } else {
                fprintf(out, "return;\n");
            }
            break;

        case NODE_IF:
            emit_indent(out, indent);
            traduzir_if(out, node, indent);
            break;

        case NODE_WHILE:
            emit_indent(out, indent);
            fprintf(out, "while (");
            traduzir_expressao(out, node->while_stmt.condition);
            fprintf(out, ") ");
            traduzir_corpo_comando(out, node->while_stmt.body, indent);
            if (node->while_stmt.body && node->while_stmt.body->type == NODE_BLOCK) {
                fprintf(out, "\n");
            }
            break;

        case NODE_DO_WHILE:
            emit_indent(out, indent);
            fprintf(out, "do ");
            traduzir_corpo_comando(out, node->do_while_stmt.body, indent);
            if (node->do_while_stmt.body && node->do_while_stmt.body->type == NODE_BLOCK) {
                fprintf(out, " while (");
            } else {
                emit_indent(out, indent);
                fprintf(out, "while (");
            }
            traduzir_expressao(out, node->do_while_stmt.condition);
            fprintf(out, ");\n");
            break;

        case NODE_FOR:
            emit_indent(out, indent);
            traduzir_for(out, node, indent);
            break;

        case NODE_SWITCH: {
            emit_indent(out, indent);
            fprintf(out, "switch (");
            traduzir_expressao(out, node->switch_stmt.expr);
            fprintf(out, ") {\n");
            for (const ASTNode *c = node->switch_stmt.cases; c; c = c->next) {
                traduzir_comando(out, c, indent + 1);
            }
            emit_indent(out, indent);
            fprintf(out, "}\n");
            break;
        }

        case NODE_CASE:
            emit_indent(out, indent);
            fprintf(out, "case ");
            traduzir_expressao(out, node->case_stmt.value);
            fprintf(out, ":\n");
            for (const ASTNode *s = node->case_stmt.statements; s; s = s->next) {
                traduzir_comando(out, s, indent + 1);
            }
            break;

        case NODE_DEFAULT:
            emit_indent(out, indent);
            fprintf(out, "default:\n");
            for (const ASTNode *s = node->default_stmt.statements; s; s = s->next) {
                traduzir_comando(out, s, indent + 1);
            }
            break;

        default:
            break;
    }
}
