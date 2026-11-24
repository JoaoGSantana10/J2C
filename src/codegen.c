#include "codegen.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Estado do gerador: sinaliza falha (de AST ou de escrita) sem usar globais
 * espalhadas; é reiniciado a cada chamada de codegen_gerar. */
static int g_erro = 0;

static void codegen_indent(FILE *saida, int nivel);
static void codegen_no(const ASTNode *no, FILE *saida, int nivel);
static void codegen_lista(const ASTNode *lista, FILE *saida, int nivel);
static void codegen_erro(int linha, const char *mensagem);
static void emitir(FILE *saida, int nivel, const char *fmt, ...);
static void codegen_comentario(FILE *saida, int nivel, const char *texto);
static void codegen_abre_funcao(FILE *saida, const char *assinatura);
static void codegen_fecha_funcao(FILE *saida, int nivel);

/* ---------------------------- erros -------------------------------- */

static void codegen_erro(int linha, const char *mensagem)
{
    g_erro = 1;
    if (linha > 0)
        fprintf(stderr, "Erro de geração de código na linha %d: %s\n",
                linha, mensagem);
    else
        fprintf(stderr, "Erro de geração de código: %s\n", mensagem);
}

/* ---------------------------- escrita ------------------------------ */

static void codegen_indent(FILE *saida, int nivel)
{
    int i;
    for (i = 0; i < nivel; i++) {
        if (fputs("    ", saida) == EOF) {
            codegen_erro(0, "impossível escrever no arquivo de saída");
            return;
        }
    }
}

/* Única porta de saída: indenta, formata e detecta falha de escrita. */
static void emitir(FILE *saida, int nivel, const char *fmt, ...)
{
    va_list args;

    if (g_erro)
        return;

    codegen_indent(saida, nivel);

    va_start(args, fmt);
    if (vfprintf(saida, fmt, args) < 0)
        codegen_erro(0, "impossível escrever no arquivo de saída");
    va_end(args);
}

static void codegen_comentario(FILE *saida, int nivel, const char *texto)
{
    emitir(saida, nivel, "/* %s */\n", texto);
}

static void codegen_abre_funcao(FILE *saida, const char *assinatura)
{
    emitir(saida, 0, "%s\n{\n", assinatura);
}

static void codegen_fecha_funcao(FILE *saida, int nivel)
{
    emitir(saida, nivel, "}\n");
}

/* --------------------------- percurso ------------------------------ */

static void codegen_lista(const ASTNode *lista, FILE *saida, int nivel)
{
    const ASTNode *n;
    for (n = lista; n != NULL && !g_erro; n = n->next)
        codegen_no(n, saida, nivel);
}

static void codegen_no(const ASTNode *no, FILE *saida, int nivel)
{
    if (g_erro)
        return;

    if (no == NULL) {
        codegen_erro(0, "nó de AST nulo");
        return;
    }

    switch (no->type) {

    case NODE_PROGRAM:
        codegen_abre_funcao(saida, "int main(void)");
        codegen_lista(no->program.body, saida, nivel + 1);
        emitir(saida, nivel + 1, "return 0;\n");
        codegen_fecha_funcao(saida, nivel);
        break;

    case NODE_CLASS:
        if (no->class_decl.name == NULL || no->class_decl.name[0] == '\0') {
            codegen_erro(no->lineno, "classe sem nome");
            return;
        }
        /* C não tem classes: por ora só registramos o nome e
         * traduzimos os membros no mesmo nível. */
        emitir(saida, nivel, "/* classe %s */\n", no->class_decl.name);
        codegen_lista(no->class_decl.members, saida, nivel);
        break;

    default:
        /* Valor fora do enum NodeType: AST corrompida ou versão errada. */
        if ((int)no->type < (int)NODE_PROGRAM ||
            (int)no->type > (int)NODE_LITERAL_BOOL) {
            codegen_erro(no->lineno, "tipo de nó de AST desconhecido");
            break;
        }
        /* Nó válido, mas ainda não implementado: falha clara, nunca
         * comportamento indefinido. As próximas issues adicionam um
         * `case` por nó aqui. */
        codegen_erro(no->lineno, "nó de AST não suportado");
        break;
    }
}

/* --------------------------- API pública --------------------------- */

int codegen_gerar(const ASTNode *raiz, FILE *saida)
{
    g_erro = 0;

    if (raiz == NULL) {
        codegen_erro(0, "AST nula");
        return CODEGEN_ERRO;
    }
    if (saida == NULL) {
        codegen_erro(0, "fluxo de saída nulo");
        return CODEGEN_ERRO;
    }
    if (raiz->type != NODE_PROGRAM) {
        codegen_erro(raiz->lineno, "a raiz da AST deve ser NODE_PROGRAM");
        return CODEGEN_ERRO;
    }

    codegen_comentario(saida, 0, "Gerado automaticamente pelo compilador J2C");
    emitir(saida, 0, "#include <stdio.h>\n");
    emitir(saida, 0, "#include <stdlib.h>\n\n");

    codegen_no(raiz, saida, 0);

    if (!g_erro && fflush(saida) == EOF)
        codegen_erro(0, "impossível escrever no arquivo de saída");

    return g_erro ? CODEGEN_ERRO : CODEGEN_OK;
}

int codegen_gerar_arquivo(const ASTNode *raiz, const char *caminho)
{
    size_t tam;
    char *tmp;
    FILE *f;
    int rc;

    if (caminho == NULL) {
        codegen_erro(0, "caminho de saída nulo");
        return CODEGEN_ERRO;
    }

    tam = strlen(caminho) + 5; /* ".tmp" + '\0' */
    tmp = malloc(tam);
    if (tmp == NULL) {
        codegen_erro(0, "memória insuficiente");
        return CODEGEN_ERRO;
    }
    snprintf(tmp, tam, "%s.tmp", caminho);

    f = fopen(tmp, "w");
    if (f == NULL) {
        codegen_erro(0, "não foi possível criar o arquivo de saída");
        free(tmp);
        return CODEGEN_ERRO;
    }

    rc = codegen_gerar(raiz, f);

    if (fclose(f) == EOF && rc == CODEGEN_OK) {
        codegen_erro(0, "impossível escrever no arquivo de saída");
        rc = CODEGEN_ERRO;
    }

    if (rc == CODEGEN_OK && rename(tmp, caminho) != 0) {
        codegen_erro(0, "não foi possível finalizar o arquivo de saída");
        rc = CODEGEN_ERRO;
    }
    if (rc != CODEGEN_OK)
        remove(tmp);

    free(tmp);
    return rc;
}
