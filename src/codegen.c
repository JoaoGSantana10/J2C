#include "codegen.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Estado de uma geração. Fica em uma struct (e não em globais) para o
 * gerador ser reentrante. */
typedef struct {
    FILE *saida;
    int erro;            /* 1 depois da primeira falha (AST ou escrita) */
    int tem_main;        /* já foi gerada uma main a partir do Java */
    int em_main;         /* dentro do corpo da main (return -> return 0) */
    int retorno_void;    /* dentro de uma função de retorno void */
} Ctx;

static void gen_no(Ctx *c, const ASTNode *no, int nivel);
static void gen_lista(Ctx *c, const ASTNode *lista, int nivel);

/* ---------------------------- erros -------------------------------- */

static void gen_erro(Ctx *c, int linha, const char *fmt, ...)
{
    va_list args;

    /* Só a primeira falha é relatada; as seguintes são consequência. */
    if (c->erro)
        return;
    c->erro = 1;

    if (linha > 0)
        fprintf(stderr, "Erro de geração de código na linha %d: ", linha);
    else
        fprintf(stderr, "Erro de geração de código: ");

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

/* ---------------------------- escrita ------------------------------ */

/* Única porta de saída: indenta, formata e detecta falha de escrita. */
static void emitir(Ctx *c, int nivel, const char *fmt, ...)
{
    va_list args;
    int i;

    if (c->erro)
        return;

    for (i = 0; i < nivel; i++) {
        if (fputs("    ", c->saida) == EOF) {
            gen_erro(c, 0, "impossível escrever no arquivo de saída");
            return;
        }
    }

    va_start(args, fmt);
    if (vfprintf(c->saida, fmt, args) < 0)
        gen_erro(c, 0, "impossível escrever no arquivo de saída");
    va_end(args);
}

/* ----------------------------- tipos ------------------------------- */

const char *codegen_tipo(const char *tipo_java)
{
    static const struct {
        const char *java;
        const char *c;
    } mapa[] = {
        {"int",     "int"},
        {"long",    "long long"},
        {"float",   "float"},
        {"double",  "double"},
        {"boolean", "bool"},
        {"char",    "char"},
        {"void",    "void"},
        {"String",  "char *"},
    };
    size_t i;

    if (tipo_java == NULL)
        return NULL;
    for (i = 0; i < sizeof mapa / sizeof mapa[0]; i++) {
        if (strcmp(tipo_java, mapa[i].java) == 0)
            return mapa[i].c;
    }
    return NULL;
}

/* Escreve "<tipo> <nome>" (ou "<tipo> *<nome>" para arrays), sem espaço
 * extra quando o tipo C já termina em '*' (ex.: "char *"). */
static void emitir_declarador(Ctx *c, const char *tipo_c, int is_array,
                              const char *nome)
{
    size_t n = strlen(tipo_c);
    int termina_em_ptr = (n > 0 && tipo_c[n - 1] == '*');

    emitir(c, 0, "%s%s%s%s", tipo_c, termina_em_ptr ? "" : " ",
           is_array ? "*" : "", nome);
}

/* Palavras que não podem ser nome em C (nem em conflito com <stdbool.h>). */
static int palavra_reservada_c(const char *nome)
{
    static const char *const reservadas[] = {
        "auto", "break", "case", "char", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "if", "inline", "int", "long", "register", "restrict", "return",
        "short", "signed", "sizeof", "static", "struct", "switch",
        "typedef", "union", "unsigned", "void", "volatile", "while",
        "bool", "true", "false",
    };
    size_t i;

    for (i = 0; i < sizeof reservadas / sizeof reservadas[0]; i++) {
        if (strcmp(nome, reservadas[i]) == 0)
            return 1;
    }
    return 0;
}

/* Valida o nome de uma declaração. Retorna 1 se serve. */
static int nome_valido(Ctx *c, const char *nome, int linha, const char *o_que)
{
    if (nome == NULL || nome[0] == '\0') {
        gen_erro(c, linha, "%s sem nome", o_que);
        return 0;
    }
    if (palavra_reservada_c(nome)) {
        gen_erro(c, linha, "%s '%s' usa palavra reservada de C", o_que, nome);
        return 0;
    }
    return 1;
}

/* Tipo C de uma declaração, ou NULL (com erro registrado). */
static const char *tipo_c_de(Ctx *c, const char *tipo_java, int linha)
{
    const char *t;

    if (tipo_java == NULL || tipo_java[0] == '\0') {
        gen_erro(c, linha, "declaração sem tipo");
        return NULL;
    }
    t = codegen_tipo(tipo_java);
    if (t == NULL)
        gen_erro(c, linha, "tipo '%s' não suportado", tipo_java);
    return t;
}

/* ------------------------ expressões simples ----------------------- */

/* Só literais e identificadores; o resto (operadores, chamadas...) é da
 * issue de expressões. */
static void gen_expr_simples(Ctx *c, const ASTNode *e)
{
    if (e == NULL) {
        gen_erro(c, 0, "expressão nula");
        return;
    }
    switch (e->type) {
    case NODE_LITERAL_INT:
    case NODE_LITERAL_LONG:
    case NODE_LITERAL_DOUBLE:
    case NODE_LITERAL_FLOAT:
    case NODE_LITERAL_CHAR:
    case NODE_LITERAL_STRING:
    case NODE_LITERAL_BOOL:
        if (e->literal.value == NULL)
            gen_erro(c, e->lineno, "literal sem valor");
        else
            emitir(c, 0, "%s", e->literal.value);
        break;
    case NODE_IDENTIFIER:
        if (e->identifier.name == NULL)
            gen_erro(c, e->lineno, "identificador sem nome");
        else
            emitir(c, 0, "%s", e->identifier.name);
        break;
    default:
        gen_erro(c, e->lineno, "expressão não suportada");
        break;
    }
}

/* --------------------------- declarações --------------------------- */

static void gen_var_decl(Ctx *c, const ASTNode *no, int nivel)
{
    const char *tipo_c;

    if (!nome_valido(c, no->var_decl.name, no->lineno, "variável"))
        return;
    tipo_c = tipo_c_de(c, no->var_decl.type_name, no->lineno);
    if (tipo_c == NULL)
        return;
    if (strcmp(tipo_c, "void") == 0) {
        gen_erro(c, no->lineno, "variável '%s' não pode ser void",
                 no->var_decl.name);
        return;
    }

    emitir(c, nivel, "");
    emitir_declarador(c, tipo_c, 0, no->var_decl.name);
    if (no->var_decl.init != NULL) {
        emitir(c, 0, " = ");
        gen_expr_simples(c, no->var_decl.init);
    }
    emitir(c, 0, ";\n");
}

static void gen_param(Ctx *c, const ASTNode *no)
{
    const char *tipo_c;

    if (no->param.type_name == NULL || no->param.type_name[0] == '\0') {
        gen_erro(c, no->lineno, "parâmetro sem tipo");
        return;
    }
    if (!nome_valido(c, no->param.name, no->lineno, "parâmetro"))
        return;
    tipo_c = tipo_c_de(c, no->param.type_name, no->lineno);
    if (tipo_c == NULL)
        return;
    if (strcmp(tipo_c, "void") == 0) {
        gen_erro(c, no->lineno, "parâmetro '%s' não pode ser void",
                 no->param.name);
        return;
    }
    emitir_declarador(c, tipo_c, no->param.is_array, no->param.name);
}

static void gen_params(Ctx *c, const ASTNode *params)
{
    const ASTNode *p;

    if (params == NULL) {
        emitir(c, 0, "void");
        return;
    }
    for (p = params; p != NULL && !c->erro; p = p->next) {
        if (p->type != NODE_PARAM) {
            gen_erro(c, p->lineno, "esperado um parâmetro");
            return;
        }
        if (p != params)
            emitir(c, 0, ", ");
        gen_param(c, p);
    }
}

/* É o `public static void main(String[] args)` do Java? */
static int eh_main_java(const ASTNode *f)
{
    const ASTNode *p = f->func_decl.params;

    return f->func_decl.name != NULL &&
           strcmp(f->func_decl.name, "main") == 0 &&
           f->func_decl.return_type != NULL &&
           strcmp(f->func_decl.return_type, "void") == 0 &&
           p != NULL && p->next == NULL && p->type == NODE_PARAM &&
           p->param.is_array && p->param.type_name != NULL &&
           strcmp(p->param.type_name, "String") == 0;
}

/* Valida o cabeçalho e devolve o tipo de retorno em C (ou NULL). */
static const char *cabecalho_funcao(Ctx *c, const ASTNode *f)
{
    if (!nome_valido(c, f->func_decl.name, f->lineno, "função"))
        return NULL;
    return tipo_c_de(c, f->func_decl.return_type, f->lineno);
}

static void gen_prototipo(Ctx *c, const ASTNode *f)
{
    const char *ret = cabecalho_funcao(c, f);

    if (ret == NULL)
        return;
    emitir_declarador(c, ret, 0, f->func_decl.name);
    emitir(c, 0, "(");
    gen_params(c, f->func_decl.params);
    emitir(c, 0, ");\n");
}

static void gen_func_decl(Ctx *c, const ASTNode *f, int nivel)
{
    const char *ret = cabecalho_funcao(c, f);
    int main_java = 0;
    int ret_void_ant = c->retorno_void;
    int em_main_ant = c->em_main;

    if (ret == NULL)
        return;
    if (f->func_decl.body == NULL || f->func_decl.body->type != NODE_BLOCK) {
        gen_erro(c, f->lineno, "função '%s' sem corpo", f->func_decl.name);
        return;
    }

    if (strcmp(f->func_decl.name, "main") == 0) {
        /* Decisão desta versão: só aceitamos main(String[] args), gerada
         * como int main(void). Os args são ignorados. */
        if (!eh_main_java(f)) {
            gen_erro(c, f->lineno,
                     "assinatura de main não suportada (use "
                     "public static void main(String[] args))");
            return;
        }
        if (c->tem_main) {
            gen_erro(c, f->lineno, "mais de uma função main");
            return;
        }
        c->tem_main = 1;
        main_java = 1;
    }

    c->em_main = main_java;
    c->retorno_void = (strcmp(ret, "void") == 0) && !main_java;

    if (main_java)
        emitir(c, nivel, "int main(void)\n");
    else {
        emitir(c, nivel, "");
        emitir_declarador(c, ret, 0, f->func_decl.name);
        emitir(c, 0, "(");
        gen_params(c, f->func_decl.params);
        emitir(c, 0, ")\n");
    }
    emitir(c, nivel, "{\n");
    gen_lista(c, f->func_decl.body->block.statements, nivel + 1);
    if (main_java)
        emitir(c, nivel + 1, "return 0;\n");
    emitir(c, nivel, "}\n");

    c->retorno_void = ret_void_ant;
    c->em_main = em_main_ant;
}

static void gen_return(Ctx *c, const ASTNode *no, int nivel)
{
    const ASTNode *e = no->return_stmt.expr;

    if (c->em_main) {
        /* main do Java é void; a do C devolve int. */
        if (e != NULL) {
            gen_erro(c, no->lineno, "main não pode retornar valor");
            return;
        }
        emitir(c, nivel, "return 0;\n");
        return;
    }
    if (c->retorno_void && e != NULL) {
        gen_erro(c, no->lineno, "função void não pode retornar valor");
        return;
    }
    if (!c->retorno_void && e == NULL) {
        gen_erro(c, no->lineno, "return sem valor em função não-void");
        return;
    }

    if (e == NULL) {
        emitir(c, nivel, "return;\n");
    } else {
        emitir(c, nivel, "return ");
        gen_expr_simples(c, e);
        emitir(c, 0, ";\n");
    }
}

/* --------------------------- classe -------------------------------- */

/* Nomes de campos e funções dividem o escopo global do C. */
static int nome_repetido(Ctx *c, const ASTNode *membros)
{
    const ASTNode *a, *b;
    const char *na, *nb;

    for (a = membros; a != NULL; a = a->next) {
        na = (a->type == NODE_VAR_DECL) ? a->var_decl.name
           : (a->type == NODE_FUNC_DECL) ? a->func_decl.name : NULL;
        if (na == NULL)
            continue;
        for (b = a->next; b != NULL; b = b->next) {
            nb = (b->type == NODE_VAR_DECL) ? b->var_decl.name
               : (b->type == NODE_FUNC_DECL) ? b->func_decl.name : NULL;
            if (nb != NULL && strcmp(na, nb) == 0) {
                gen_erro(c, b->lineno,
                         "nome '%s' repetido (C não tem sobrecarga)", nb);
                return 1;
            }
        }
    }
    return 0;
}

/* Decisão da equipe (primeira versão): campos viram variáveis globais e
 * métodos viram funções. Ordem no C: campos, protótipos, definições. */
static void gen_class(Ctx *c, const ASTNode *no, int nivel)
{
    const ASTNode *m;
    int ha_campos = 0, ha_funcoes = 0;

    if (no->class_decl.name == NULL || no->class_decl.name[0] == '\0') {
        gen_erro(c, no->lineno, "classe sem nome");
        return;
    }
    emitir(c, nivel, "/* Classe %s */\n", no->class_decl.name);

    if (nome_repetido(c, no->class_decl.members))
        return;

    for (m = no->class_decl.members; m != NULL && !c->erro; m = m->next) {
        if (m->type == NODE_VAR_DECL) {
            gen_var_decl(c, m, nivel);
            ha_campos = 1;
        } else if (m->type == NODE_FUNC_DECL) {
            ha_funcoes = 1;
        } else {
            gen_erro(c, m->lineno, "membro de classe não suportado");
        }
    }

    if (ha_funcoes) {
        int protos = 0;

        for (m = no->class_decl.members; m != NULL && !c->erro; m = m->next) {
            if (m->type != NODE_FUNC_DECL || eh_main_java(m))
                continue;
            if (!protos) {
                if (ha_campos)
                    emitir(c, 0, "\n");
                protos = 1;
            }
            emitir(c, nivel, "");
            gen_prototipo(c, m);
        }
        for (m = no->class_decl.members; m != NULL && !c->erro; m = m->next) {
            if (m->type != NODE_FUNC_DECL)
                continue;
            emitir(c, 0, "\n");
            gen_func_decl(c, m, nivel);
        }
    }
}

/* --------------------------- percurso ------------------------------ */

static void gen_lista(Ctx *c, const ASTNode *lista, int nivel)
{
    const ASTNode *n;

    for (n = lista; n != NULL && !c->erro; n = n->next)
        gen_no(c, n, nivel);
}

static void gen_no(Ctx *c, const ASTNode *no, int nivel)
{
    if (c->erro)
        return;

    if (no == NULL) {
        gen_erro(c, 0, "nó de AST nulo");
        return;
    }

    switch (no->type) {

    case NODE_PROGRAM:
        gen_lista(c, no->program.body, nivel);
        /* Sem main no Java, geramos uma vazia para o C continuar válido. */
        if (!c->erro && !c->tem_main) {
            emitir(c, 0, "\nint main(void)\n{\n");
            emitir(c, 1, "return 0;\n");
            emitir(c, 0, "}\n");
        }
        break;

    case NODE_CLASS:
        gen_class(c, no, nivel);
        break;

    case NODE_VAR_DECL:
        gen_var_decl(c, no, nivel);
        break;

    case NODE_FUNC_DECL:
        gen_func_decl(c, no, nivel);
        break;

    case NODE_BLOCK:
        emitir(c, nivel, "{\n");
        gen_lista(c, no->block.statements, nivel + 1);
        emitir(c, nivel, "}\n");
        break;

    case NODE_RETURN:
        gen_return(c, no, nivel);
        break;

    default:
        /* Valor fora do enum NodeType: AST corrompida ou versão errada. */
        if ((int)no->type < (int)NODE_PROGRAM ||
            (int)no->type >= (int)NODE_COUNT) {
            gen_erro(c, no->lineno, "tipo de nó de AST desconhecido");
            break;
        }
        /* Nó válido, mas ainda não implementado: falha clara, nunca
         * comportamento indefinido. As próximas issues adicionam um
         * `case` por nó aqui. */
        gen_erro(c, no->lineno, "nó de AST não suportado");
        break;
    }
}

/* --------------------------- API pública --------------------------- */

int codegen_gerar(const ASTNode *raiz, FILE *saida)
{
    Ctx c;

    memset(&c, 0, sizeof c);
    c.saida = saida;

    if (raiz == NULL) {
        gen_erro(&c, 0, "AST nula");
        return CODEGEN_ERRO;
    }
    if (saida == NULL) {
        gen_erro(&c, 0, "fluxo de saída nulo");
        return CODEGEN_ERRO;
    }
    if (raiz->type != NODE_PROGRAM) {
        gen_erro(&c, raiz->lineno, "a raiz da AST deve ser NODE_PROGRAM");
        return CODEGEN_ERRO;
    }

    emitir(&c, 0, "/* Gerado automaticamente pelo compilador J2C */\n");
    emitir(&c, 0, "#include <stdio.h>\n");
    emitir(&c, 0, "#include <stdlib.h>\n");
    emitir(&c, 0, "#include <stdbool.h>\n\n");

    gen_no(&c, raiz, 0);

    if (!c.erro && fflush(saida) == EOF)
        gen_erro(&c, 0, "impossível escrever no arquivo de saída");

    return c.erro ? CODEGEN_ERRO : CODEGEN_OK;
}

int codegen_gerar_arquivo(const ASTNode *raiz, const char *caminho)
{
    size_t tam;
    char *tmp;
    FILE *f;
    int rc;

    if (caminho == NULL) {
        fprintf(stderr, "Erro de geração de código: caminho de saída nulo\n");
        return CODEGEN_ERRO;
    }

    tam = strlen(caminho) + 5; /* ".tmp" + '\0' */
    tmp = malloc(tam);
    if (tmp == NULL) {
        fprintf(stderr, "Erro de geração de código: memória insuficiente\n");
        return CODEGEN_ERRO;
    }
    snprintf(tmp, tam, "%s.tmp", caminho);

    /* "wx": falha se o .tmp já existir, para nunca sobrescrever um arquivo
     * que não é nosso. */
    f = fopen(tmp, "wx");
    if (f == NULL) {
        fprintf(stderr, "Erro de geração de código: não foi possível criar "
                        "'%s' (já existe ou sem permissão)\n", tmp);
        free(tmp);
        return CODEGEN_ERRO;
    }

    rc = codegen_gerar(raiz, f);

    if (fclose(f) == EOF && rc == CODEGEN_OK) {
        fprintf(stderr, "Erro de geração de código: "
                        "impossível escrever no arquivo de saída\n");
        rc = CODEGEN_ERRO;
    }

    if (rc == CODEGEN_OK && rename(tmp, caminho) != 0) {
        /* No Windows o rename falha se o destino já existe: remove e
         * tenta de novo. */
        remove(caminho);
        if (rename(tmp, caminho) != 0) {
            fprintf(stderr, "Erro de geração de código: "
                            "não foi possível finalizar o arquivo de saída\n");
            rc = CODEGEN_ERRO;
        }
    }
    if (rc != CODEGEN_OK)
        remove(tmp);

    free(tmp);
    return rc;
}
