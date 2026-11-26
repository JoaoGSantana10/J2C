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

        case NODE_ASSIGN: {
            emit_indent(out, indent);
            const char *op_str = "=";
            if (node->assign.op == AST_ASG_PLUS_ASSIGN) {
                op_str = "+=";
            } else if (node->assign.op == AST_ASG_MINUS_ASSIGN) {
                op_str = "-=";
            }
            fprintf(out, "%s %s ", node->assign.var_name, op_str);
            traduzir_expressao(out, node->assign.expr);
            fprintf(out, ";\n");
            break;
        }

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

        default:
            break;
    }
}
