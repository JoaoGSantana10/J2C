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

        default:
            break;
    }
}
