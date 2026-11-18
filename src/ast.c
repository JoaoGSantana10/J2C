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

// Assumi a posse dos dois pedaços e os libera
char* ast_join_qualified(char *left, char *right) {
    size_t len = strlen(left) + 1 + strlen(right) + 1;
    char *joined = malloc(len);
    if (!joined) {
        fprintf(stderr, "Falha ao tentar montar um nome qualificado\n");
        exit(EXIT_FAILURE);
    }
    strcpy(joined,left );
    strcat(joined, ".");
    strcat(joined, right);
    free(left);
    free(right);
    return joined;
}

void ast_set_modifiers(ASTNode *node, int modifiers) {
    switch (node->type) {
        case NODE_VAR_DECL:
            node->var_decl.modifiers = modifiers;
            break;
        case NODE_FUNC_DECL:
            node->func_decl.modifiers = modifiers;
            break;
        default:
            break;
    }
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

ASTNode* ast_create_assign(AssignOpType op, char *var_name, ASTNode *expr, int lineno) {
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

ASTNode* ast_create_bin_op(BinaryOpType op, ASTNode *left, ASTNode *right, int lineno) {
    ASTNode *node = ast_create_node(NODE_BIN_OP, lineno);
    node->bin_op.op = op;
    node->bin_op.left = left;
    node->bin_op.right = right;
    return node;
}

ASTNode* ast_create_unary_op(UnaryOpType op, int is_postfix, ASTNode *operand, int lineno) {
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

/* ------------------------------------------------------------------
 * Impressao da AST
 *
 * Imprime a arvore com indentacao por nivel, para validar visualmente
 * a construcao feita pelas acoes do parser. Exemplo:
 *
 *   Programa
 *   └── Classe A
 *       └── Funcao main
 *           ├── Param String[] args
 *           └── DeclVar int n
 *               └── Literal 1
 *
 * Cada no imprime um rotulo (tipo do no + lexema) e depois seus filhos.
 * Os filhos de um no sao descritos por "slots" (ChildSlot):
 *   - slot sem rotulo: cada no da lista vira um filho direto;
 *   - slot com rotulo: cria uma linha extra (ex.: "Condicao", "Corpo")
 *     que agrupa a lista, deixando claro o papel de cada sub-arvore.
 * Slots com lista vazia (NULL) nao sao impressos.
 * ------------------------------------------------------------------ */

#define AST_MAX_SLOTS 4

#define BRANCH_MID   "├── "
#define BRANCH_LAST  "└── "
#define INDENT_MID   "│   "
#define INDENT_LAST  "    "

typedef struct {
    const char *label;      /* NULL = os nos da lista sao filhos diretos */
    const ASTNode *list;    /* lista encadeada por ->next (pode ser NULL) */
} ChildSlot;

/* Evita passar NULL para printf("%s") */
static const char* safe_str(const char *s) {
    return s ? s : "?";
}

static const char* bin_op_str(BinaryOpType op) {
    switch (op) {
        case AST_OP_ADD: return "+";
        case AST_OP_SUB: return "-";
        case AST_OP_MUL: return "*";
        case AST_OP_DIV: return "/";
        case AST_OP_MOD: return "%";
        case AST_OP_EQ:  return "==";
        case AST_OP_NE:  return "!=";
        case AST_OP_LT:  return "<";
        case AST_OP_GT:  return ">";
        case AST_OP_LE:  return "<=";
        case AST_OP_GE:  return ">=";
        case AST_OP_AND: return "&&";
        case AST_OP_OR:  return "||";
    }
    return "?";
}

static const char* unary_op_str(UnaryOpType op) {
    switch (op) {
        case AST_UN_INC: return "++";
        case AST_UN_DEC: return "--";
        case AST_UN_NEG: return "-";
        case AST_UN_NOT: return "!";
    }
    return "?";
}

static const char* assign_op_str(AssignOpType op) {
    switch (op) {
        case AST_ASG_ASSIGN:       return "=";
        case AST_ASG_PLUS_ASSIGN:  return "+=";
        case AST_ASG_MINUS_ASSIGN: return "-=";
    }
    return "?";
}

static int is_literal(const ASTNode *node) {
    if (!node) return 0;
    switch (node->type) {
        case NODE_LITERAL_INT:
        case NODE_LITERAL_DOUBLE:
        case NODE_LITERAL_FLOAT:
        case NODE_LITERAL_LONG:
        case NODE_LITERAL_CHAR:
        case NODE_LITERAL_STRING:
        case NODE_LITERAL_BOOL:
            return 1;
        default:
            return 0;
    }
}

/* Imprime o rotulo do no (sem quebra de linha) */
static void print_label(const ASTNode *node) {
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Programa");
            break;
        case NODE_CLASS:
            printf("Classe %s", safe_str(node->class_decl.name));
            break;
        case NODE_VAR_DECL:
            printf("DeclVar %s %s", safe_str(node->var_decl.type_name), safe_str(node->var_decl.name));
            break;
        case NODE_FUNC_DECL:
            printf("Funcao %s", safe_str(node->func_decl.name));
            break;
        case NODE_PARAM:
            printf("Param %s%s %s", safe_str(node->param.type_name),
                   node->param.is_array ? "[]" : "", safe_str(node->param.name));
            break;
        case NODE_BLOCK:
            printf("Bloco");
            break;
        case NODE_ASSIGN:
            printf("Atribuicao %s %s", safe_str(node->assign.var_name), assign_op_str(node->assign.op));
            break;
        case NODE_EXPR_STMT:
            printf("ComandoExpr");
            break;
        case NODE_IF:
            printf("If");
            break;
        case NODE_WHILE:
            printf("While");
            break;
        case NODE_DO_WHILE:
            printf("DoWhile");
            break;
        case NODE_FOR:
            printf("For");
            break;
        case NODE_SWITCH:
            printf("Switch");
            break;
        case NODE_CASE:
            /* O valor do case e sempre um literal na gramatica: mostra direto no rotulo */
            if (is_literal(node->case_stmt.value)) {
                printf("Case %s", safe_str(node->case_stmt.value->literal.value));
            } else {
                printf("Case");
            }
            break;
        case NODE_DEFAULT:
            printf("Default");
            break;
        case NODE_RETURN:
            printf("Return");
            break;
        case NODE_BREAK:
            printf("Break");
            break;
        case NODE_CONTINUE:
            printf("Continue");
            break;
        case NODE_BIN_OP:
            printf("OpBinario %s", bin_op_str(node->bin_op.op));
            break;
        case NODE_UNARY_OP:
            printf("OpUnario %s", unary_op_str(node->unary_op.op));
            if (node->unary_op.op == AST_UN_INC || node->unary_op.op == AST_UN_DEC) {
                printf(" (%s)", node->unary_op.is_postfix ? "posfixo" : "prefixo");
            }
            break;
        case NODE_CALL:
            printf("Chamada %s", safe_str(node->call.func_name));
            break;
        case NODE_NEW:
            printf("New %s(%s)", safe_str(node->new_expr.class_name),
                   node->new_expr.arg_name ? node->new_expr.arg_name : "");
            break;
        case NODE_IDENTIFIER:
            printf("Id %s", safe_str(node->identifier.name));
            break;
        case NODE_LITERAL_INT:
        case NODE_LITERAL_DOUBLE:
        case NODE_LITERAL_FLOAT:
        case NODE_LITERAL_LONG:
        case NODE_LITERAL_CHAR:
        case NODE_LITERAL_STRING:
        case NODE_LITERAL_BOOL:
            printf("Literal %s", safe_str(node->literal.value));
            break;
    }
}

/* Preenche os slots de filhos do no e retorna quantos foram usados */
static int collect_children(const ASTNode *node, ChildSlot slots[AST_MAX_SLOTS]) {
    int n = 0;

    switch (node->type) {
        case NODE_PROGRAM:
            slots[n++] = (ChildSlot){ NULL, node->program.body };
            break;
        case NODE_CLASS:
            slots[n++] = (ChildSlot){ NULL, node->class_decl.members };
            break;
        case NODE_VAR_DECL:
            slots[n++] = (ChildSlot){ NULL, node->var_decl.init };
            break;
        case NODE_FUNC_DECL: {
            /* O corpo de uma funcao e sempre um bloco: os comandos aparecem
             * direto abaixo da funcao, sem a linha "Bloco" no meio. */
            const ASTNode *body = node->func_decl.body;
            slots[n++] = (ChildSlot){ NULL, node->func_decl.params };
            slots[n++] = (ChildSlot){ NULL, (body && body->type == NODE_BLOCK) ? body->block.statements : body };
            break;
        }
        case NODE_BLOCK:
            slots[n++] = (ChildSlot){ NULL, node->block.statements };
            break;
        case NODE_ASSIGN:
            slots[n++] = (ChildSlot){ NULL, node->assign.expr };
            break;
        case NODE_EXPR_STMT:
            slots[n++] = (ChildSlot){ NULL, node->expr_stmt.expr };
            break;
        case NODE_IF:
            slots[n++] = (ChildSlot){ "Condicao", node->if_stmt.condition };
            slots[n++] = (ChildSlot){ "Entao", node->if_stmt.then_branch };
            slots[n++] = (ChildSlot){ "Senao", node->if_stmt.else_branch };
            break;
        case NODE_WHILE:
            slots[n++] = (ChildSlot){ "Condicao", node->while_stmt.condition };
            slots[n++] = (ChildSlot){ "Corpo", node->while_stmt.body };
            break;
        case NODE_DO_WHILE:
            slots[n++] = (ChildSlot){ "Corpo", node->do_while_stmt.body };
            slots[n++] = (ChildSlot){ "Condicao", node->do_while_stmt.condition };
            break;
        case NODE_FOR:
            slots[n++] = (ChildSlot){ "Init", node->for_stmt.init };
            slots[n++] = (ChildSlot){ "Condicao", node->for_stmt.condition };
            slots[n++] = (ChildSlot){ "Incremento", node->for_stmt.increment };
            slots[n++] = (ChildSlot){ "Corpo", node->for_stmt.body };
            break;
        case NODE_SWITCH:
            slots[n++] = (ChildSlot){ "Expressao", node->switch_stmt.expr };
            slots[n++] = (ChildSlot){ NULL, node->switch_stmt.cases };
            break;
        case NODE_CASE:
            if (!is_literal(node->case_stmt.value)) {
                slots[n++] = (ChildSlot){ "Valor", node->case_stmt.value };
            }
            slots[n++] = (ChildSlot){ NULL, node->case_stmt.statements };
            break;
        case NODE_DEFAULT:
            slots[n++] = (ChildSlot){ NULL, node->default_stmt.statements };
            break;
        case NODE_RETURN:
            slots[n++] = (ChildSlot){ NULL, node->return_stmt.expr };
            break;
        case NODE_BIN_OP:
            slots[n++] = (ChildSlot){ NULL, node->bin_op.left };
            slots[n++] = (ChildSlot){ NULL, node->bin_op.right };
            break;
        case NODE_UNARY_OP:
            slots[n++] = (ChildSlot){ NULL, node->unary_op.operand };
            break;
        case NODE_CALL:
            slots[n++] = (ChildSlot){ NULL, node->call.args };
            break;
        default:
            /* Folhas: BREAK, CONTINUE, NEW, IDENTIFIER e literais */
            break;
    }

    return n;
}

/* Prefixo dos filhos = prefixo atual + "│   " (ou "    " se for o ultimo) */
static char* extend_prefix(const char *prefix, int is_last) {
    const char *ext = is_last ? INDENT_LAST : INDENT_MID;
    char *result = malloc(strlen(prefix) + strlen(ext) + 1);
    if (!result) {
        fprintf(stderr, "Erro de alocacao de memoria ao imprimir a AST\n");
        exit(EXIT_FAILURE);
    }
    strcpy(result, prefix);
    strcat(result, ext);
    return result;
}

static void print_subtree(const ASTNode *node, const char *prefix, int is_last);

static void print_slots(const ChildSlot slots[], int count, const char *prefix) {
    for (int i = 0; i < count; i++) {
        if (!slots[i].list) continue;

        /* Este slot e o ultimo se nenhum slot seguinte tiver filhos */
        int has_more_slots = 0;
        for (int j = i + 1; j < count; j++) {
            if (slots[j].list) {
                has_more_slots = 1;
                break;
            }
        }

        if (slots[i].label) {
            int is_last = !has_more_slots;
            printf("%s%s%s\n", prefix, is_last ? BRANCH_LAST : BRANCH_MID, slots[i].label);

            char *child_prefix = extend_prefix(prefix, is_last);
            ChildSlot group = { NULL, slots[i].list };
            print_slots(&group, 1, child_prefix);
            free(child_prefix);
        } else {
            for (const ASTNode *child = slots[i].list; child; child = child->next) {
                print_subtree(child, prefix, !child->next && !has_more_slots);
            }
        }
    }
}

static void print_children(const ASTNode *node, const char *prefix) {
    ChildSlot slots[AST_MAX_SLOTS];
    int count = collect_children(node, slots);
    print_slots(slots, count, prefix);
}

static void print_subtree(const ASTNode *node, const char *prefix, int is_last) {
    printf("%s%s", prefix, is_last ? BRANCH_LAST : BRANCH_MID);
    print_label(node);
    putchar('\n');

    char *child_prefix = extend_prefix(prefix, is_last);
    print_children(node, child_prefix);
    free(child_prefix);
}

void ast_print(const ASTNode *root) {
    /* A raiz e impressa sem conector; os filhos ganham "├──" / "└──" */
    for (const ASTNode *node = root; node; node = node->next) {
        print_label(node);
        putchar('\n');
        print_children(node, "");
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
