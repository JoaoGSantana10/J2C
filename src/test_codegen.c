#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen.h"

static int falhas = 0;

#define CHECA(cond, msg) do { \
    if (cond) printf("ok    - %s\n", msg); \
    else { printf("FALHA - %s\n", msg); falhas++; } \
} while (0)

static void le_saida(FILE *f, char *buf, size_t tam)
{
    size_t n;
    rewind(f);
    n = fread(buf, 1, tam - 1, f);
    buf[n] = '\0';
}

static int existe(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (f) fclose(f);
    return f != NULL;
}

int main(void)
{
    char buf[1024];
    FILE *f;
    ASTNode *prog;

    /* 1. AST nula deve falhar sem quebrar */
    f = tmpfile();
    CHECA(codegen_gerar(NULL, f) == CODEGEN_ERRO, "AST nula retorna erro");
    fclose(f);

    /* 2. Programa + classe válidos geram o contrato mínimo */
    prog = ast_create_program(
        ast_create_class(MOD_PUBLIC, ast_safe_strdup("Main"), NULL, 1), 1);
    f = tmpfile();
    CHECA(codegen_gerar(prog, f) == CODEGEN_OK, "programa válido retorna ok");
    le_saida(f, buf, sizeof buf);
    CHECA(strstr(buf, "#include <stdio.h>") != NULL, "inclui stdio.h");
    CHECA(strstr(buf, "#include <stdlib.h>") != NULL, "inclui stdlib.h");
    CHECA(strstr(buf, "int main(void)") != NULL, "gera main");
    CHECA(strstr(buf, "return 0;") != NULL, "gera return 0");
    CHECA(strstr(buf, "classe Main") != NULL, "comenta a classe");
    fclose(f);

    /* 3. Geração em arquivo real */
    remove("teste_ok.c");
    CHECA(codegen_gerar_arquivo(prog, "teste_ok.c") == CODEGEN_OK,
          "gerar_arquivo retorna ok");
    CHECA(existe("teste_ok.c"), "arquivo final criado");
    CHECA(!existe("teste_ok.c.tmp"), "sem .tmp após sucesso");
    remove("teste_ok.c");

    /* 4. A AST não foi liberada: ainda é utilizável aqui */
    CHECA(prog->program.body != NULL && prog->program.body->type == NODE_CLASS,
          "AST intacta após a geração");
    ast_free(prog);

    /* 5. Classe sem nome falha e não deixa arquivo parcial */
    prog = ast_create_program(ast_create_class(MOD_NONE, NULL, NULL, 3), 1);
    CHECA(codegen_gerar_arquivo(prog, "teste_falha.c") == CODEGEN_ERRO,
          "classe sem nome retorna erro");
    CHECA(!existe("teste_falha.c"), "falha não deixa arquivo final");
    CHECA(!existe("teste_falha.c.tmp"), "falha não deixa .tmp");
    ast_free(prog);

    /* 6. Nó ainda não implementado gera falha clara (sem segfault) */
    prog = ast_create_program(
        ast_create_class(MOD_PUBLIC, ast_safe_strdup("Main"),
                         ast_create_break(7), 1), 1);
    f = tmpfile();
    CHECA(codegen_gerar(prog, f) == CODEGEN_ERRO,
          "nó não suportado retorna erro");
    fclose(f);
    ast_free(prog);

    /* 7. Raiz que não é NODE_PROGRAM é rejeitada */
    prog = ast_create_break(1);
    f = tmpfile();
    CHECA(codegen_gerar(prog, f) == CODEGEN_ERRO, "raiz inválida retorna erro");
    fclose(f);
    ast_free(prog);

    printf(falhas ? "\n%d falha(s)\n" : "\ntodos os testes passaram\n", falhas);
    return falhas ? 1 : 0;
}
