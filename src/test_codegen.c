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

static char *S(const char *s) { return ast_safe_strdup(s); }

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

/* Gera `prog` em um tmpfile e devolve o resultado; o texto vai para buf. */
static int gera(const ASTNode *prog, char *buf, size_t tam)
{
    FILE *f = tmpfile();
    int rc = codegen_gerar(prog, f);
    le_saida(f, buf, tam);
    fclose(f);
    return rc;
}

/* Programa com uma classe "Main" contendo os membros dados. */
static ASTNode *programa(ASTNode *membros)
{
    return ast_create_program(
        ast_create_class(MOD_PUBLIC, S("Main"), membros, 1), 1);
}

static ASTNode *lit_int(const char *v)
{
    return ast_create_literal(NODE_LITERAL_INT, S(v), 1);
}

/* Compila o C gerado com o gcc. Retorna 1 se compilou sem avisos. */
static int compila(const ASTNode *prog)
{
    int rc;
    remove("teste_gcc.c");
    if (codegen_gerar_arquivo(prog, "teste_gcc.c") != CODEGEN_OK)
        return 0;
    rc = system("gcc -Wall -Wno-unused-variable -Werror -c teste_gcc.c "
                "-o teste_gcc.o 2> /dev/null");
    remove("teste_gcc.c");
    remove("teste_gcc.o");
    return rc == 0;
}

/* ---- casos que devem falhar: AST montada com `membros` ------------- */
static int falha_com(ASTNode *membros)
{
    char buf[4096];
    ASTNode *prog = programa(membros);
    int rc = gera(prog, buf, sizeof buf);
    ast_free(prog);
    return rc == CODEGEN_ERRO;
}

static void testes_tipos(void)
{
    CHECA(strcmp(codegen_tipo("int"), "int") == 0, "int -> int");
    CHECA(strcmp(codegen_tipo("long"), "long long") == 0, "long -> long long");
    CHECA(strcmp(codegen_tipo("float"), "float") == 0, "float -> float");
    CHECA(strcmp(codegen_tipo("double"), "double") == 0, "double -> double");
    CHECA(strcmp(codegen_tipo("boolean"), "bool") == 0, "boolean -> bool");
    CHECA(strcmp(codegen_tipo("char"), "char") == 0, "char -> char");
    CHECA(strcmp(codegen_tipo("void"), "void") == 0, "void -> void");
    CHECA(strcmp(codegen_tipo("String"), "char *") == 0, "String -> char *");
    CHECA(codegen_tipo("Pessoa") == NULL, "tipo de classe retorna NULL");
    CHECA(codegen_tipo("") == NULL, "tipo vazio retorna NULL");
    CHECA(codegen_tipo(NULL) == NULL, "tipo NULL retorna NULL");
}

static void testes_basicos(void)
{
    char buf[4096];
    FILE *f;
    ASTNode *prog;

    f = tmpfile();
    CHECA(codegen_gerar(NULL, f) == CODEGEN_ERRO, "AST nula retorna erro");
    fclose(f);

    /* Classe vazia: contrato mínimo, com main sintética */
    prog = programa(NULL);
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "programa válido ok");
    CHECA(strstr(buf, "#include <stdio.h>") != NULL, "inclui stdio.h");
    CHECA(strstr(buf, "#include <stdlib.h>") != NULL, "inclui stdlib.h");
    CHECA(strstr(buf, "#include <stdbool.h>") != NULL, "inclui stdbool.h");
    CHECA(strstr(buf, "/* Classe Main */") != NULL, "comenta a classe");
    CHECA(strstr(buf, "int main(void)") != NULL, "gera main sintética");
    CHECA(strstr(buf, "return 0;") != NULL, "gera return 0");
    CHECA(compila(prog), "classe vazia compila no gcc");

    /* Arquivo real, sem .tmp sobrando; AST segue utilizável */
    remove("teste_ok.c");
    CHECA(codegen_gerar_arquivo(prog, "teste_ok.c") == CODEGEN_OK,
          "gerar_arquivo ok");
    CHECA(existe("teste_ok.c"), "arquivo final criado");
    CHECA(!existe("teste_ok.c.tmp"), "sem .tmp após sucesso");
    CHECA(prog->program.body != NULL && prog->program.body->type == NODE_CLASS,
          "AST intacta após a geração");
    remove("teste_ok.c");
    ast_free(prog);

    /* Sobrescreve um destino que já existe */
    prog = programa(NULL);
    { FILE *g = fopen("teste_dest.c", "w"); fputs("velho", g); fclose(g); }
    CHECA(codegen_gerar_arquivo(prog, "teste_dest.c") == CODEGEN_OK,
          "gerar_arquivo sobrescreve destino existente");
    remove("teste_dest.c");
    ast_free(prog);

    /* .tmp pré-existente não é sobrescrito */
    prog = programa(NULL);
    { FILE *g = fopen("teste_tmp.c.tmp", "w"); fputs("meu", g); fclose(g); }
    CHECA(codegen_gerar_arquivo(prog, "teste_tmp.c") == CODEGEN_ERRO,
          ".tmp existente faz falhar");
    {
        char lido[8] = {0};
        FILE *g = fopen("teste_tmp.c.tmp", "r");
        if (g) { if (fgets(lido, sizeof lido, g)) {} fclose(g); }
        CHECA(strcmp(lido, "meu") == 0, ".tmp existente preservado");
    }
    CHECA(!existe("teste_tmp.c"), "sem arquivo final quando .tmp existe");
    remove("teste_tmp.c.tmp");
    ast_free(prog);

    /* Raiz que não é NODE_PROGRAM */
    prog = ast_create_break(1);
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_ERRO, "raiz inválida erro");
    ast_free(prog);

    /* Tipo de nó fora do enum (NODE_COUNT) */
    prog = programa(NULL);
    prog->program.body->class_decl.members = ast_create_node(NODE_COUNT, 5);
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_ERRO, "nó NODE_COUNT erro");
    ast_free(prog);

    /* Nó ainda não implementado: falha clara */
    CHECA(falha_com(ast_create_break(7)), "nó não suportado retorna erro");
}

static void testes_variaveis(void)
{
    char buf[4096];
    ASTNode *prog;

    /* int x; double resultado = 10.5; */
    prog = programa(ast_append(
        ast_create_var_decl(MOD_NONE, S("int"), S("x"), NULL, 2),
        ast_create_var_decl(MOD_PRIVATE | MOD_STATIC, S("double"),
            S("resultado"),
            ast_create_literal(NODE_LITERAL_DOUBLE, S("10.5"), 3), 3)));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "campos geram ok");
    CHECA(strstr(buf, "\nint x;\n") != NULL, "int x;");
    CHECA(strstr(buf, "\ndouble resultado = 10.5;\n") != NULL,
          "double com inicializador");
    CHECA(strstr(buf, "private") == NULL && strstr(buf, "static") == NULL,
          "modificadores não vazam para o C");
    CHECA(compila(prog), "campos compilam no gcc");
    ast_free(prog);

    /* Todos os tipos primitivos + String */
    prog = programa(ast_append(ast_append(ast_append(ast_append(ast_append(
        ast_create_var_decl(MOD_NONE, S("long"), S("a"),
            ast_create_literal(NODE_LITERAL_LONG, S("5L"), 1), 1),
        ast_create_var_decl(MOD_NONE, S("float"), S("b"),
            ast_create_literal(NODE_LITERAL_FLOAT, S("1.5f"), 1), 1)),
        ast_create_var_decl(MOD_NONE, S("boolean"), S("c"),
            ast_create_literal(NODE_LITERAL_BOOL, S("true"), 1), 1)),
        ast_create_var_decl(MOD_NONE, S("char"), S("d"),
            ast_create_literal(NODE_LITERAL_CHAR, S("'z'"), 1), 1)),
        ast_create_var_decl(MOD_NONE, S("String"), S("e"),
            ast_create_literal(NODE_LITERAL_STRING, S("\"oi\""), 1), 1)),
        ast_create_var_decl(MOD_NONE, S("int"), S("g"), lit_int("1"), 1)));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "todos os tipos ok");
    CHECA(strstr(buf, "long long a = 5L;") != NULL, "long long");
    CHECA(strstr(buf, "float b = 1.5f;") != NULL, "float");
    CHECA(strstr(buf, "bool c = true;") != NULL, "bool");
    CHECA(strstr(buf, "char d = 'z';") != NULL, "char");
    CHECA(strstr(buf, "char *e = \"oi\";") != NULL, "String -> char *");
    CHECA(compila(prog), "todos os tipos compilam no gcc");
    ast_free(prog);

    /* Casos negativos */
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("int"), NULL, NULL, 2)),
          "variável sem nome falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("int"), S(""), NULL, 2)),
          "variável com nome vazio falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("Pessoa"), S("p"),
                                        NULL, 2)),
          "tipo desconhecido falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, NULL, S("p"), NULL, 2)),
          "variável sem tipo falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("void"), S("v"), NULL, 2)),
          "variável void falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("int"), S("register"),
                                        NULL, 2)),
          "nome igual a palavra reservada de C falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("int"), S("bool"),
                                        NULL, 2)),
          "nome 'bool' falha");
    CHECA(falha_com(ast_create_var_decl(MOD_NONE, S("int"), S("x"),
              ast_create_bin_op(AST_OP_ADD, lit_int("1"), lit_int("2"), 2), 2)),
          "inicializador complexo (fora desta issue) falha");
    CHECA(falha_com(ast_append(
              ast_create_var_decl(MOD_NONE, S("int"), S("x"), NULL, 2),
              ast_create_var_decl(MOD_NONE, S("int"), S("x"), NULL, 3))),
          "campo repetido falha");
    CHECA(falha_com(ast_append(
              ast_create_var_decl(MOD_NONE, S("int"), S("a"), lit_int("1"), 2),
              ast_create_var_decl(MOD_NONE, S("int"), S("b"),
                  ast_create_identifier(S("a"), 3), 3))),
          "campo inicializado com outro campo falha");
}

static ASTNode *func(const char *ret, const char *nome, ASTNode *params,
                     ASTNode *stmts)
{
    return ast_create_func_decl(MOD_PUBLIC, S(ret), S(nome), params,
                                ast_create_block(stmts, 1), 1);
}

static void testes_funcoes(void)
{
    char buf[4096];
    ASTNode *prog;

    /* double calcular(double x, int y) { return x; } */
    prog = programa(func("double", "calcular",
        ast_append(ast_create_param(S("double"), S("x"), 0, 1),
                   ast_create_param(S("int"), S("y"), 0, 1)),
        ast_create_return(ast_create_identifier(S("x"), 1), 1)));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "função com params ok");
    CHECA(strstr(buf, "double calcular(double x, int y);\n") != NULL,
          "gera protótipo");
    CHECA(strstr(buf, "double calcular(double x, int y)\n{\n") != NULL,
          "gera definição");
    CHECA(strstr(buf, "    return x;\n}") != NULL, "gera corpo indentado");
    CHECA(compila(prog), "função compila no gcc");
    ast_free(prog);

    /* void executar() {}  ->  void executar(void) */
    prog = programa(func("void", "executar", NULL, NULL));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "função vazia ok");
    CHECA(strstr(buf, "void executar(void)\n{\n}\n") != NULL,
          "void sem params vira (void)");
    CHECA(compila(prog), "função vazia compila no gcc");
    ast_free(prog);

    /* void f(String[] xs, String s) e return; */
    prog = programa(func("void", "f",
        ast_append(ast_create_param(S("String"), S("xs"), 1, 1),
                   ast_create_param(S("String"), S("s"), 0, 1)),
        ast_create_return(NULL, 1)));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "params String ok");
    CHECA(strstr(buf, "void f(char **xs, char *s)") != NULL,
          "String[] -> char **, String -> char *");
    CHECA(strstr(buf, "    return;\n") != NULL, "return; em função void");
    CHECA(compila(prog), "params String compilam no gcc");
    ast_free(prog);

    /* variável local e bloco aninhado */
    prog = programa(func("int", "g", NULL, ast_append(
        ast_create_var_decl(MOD_NONE, S("int"), S("r"), lit_int("2"), 1),
        ast_create_block(ast_create_return(lit_int("0"), 1), 1))));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "local e bloco ok");
    CHECA(strstr(buf, "    int r = 2;\n    {\n        return 0;\n    }\n")
          != NULL, "local e bloco aninhado indentados");
    ast_free(prog);

    /* Fields antes dos protótipos, protótipos antes das definições */
    prog = programa(ast_append(
        func("int", "um", NULL, ast_create_return(lit_int("1"), 1)),
        ast_create_var_decl(MOD_NONE, S("int"), S("campo"), NULL, 1)));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "ordem ok");
    CHECA(strstr(buf, "int campo;") < strstr(buf, "int um(void);") &&
          strstr(buf, "int um(void);") < strstr(buf, "int um(void)\n{"),
          "ordem: campos, protótipos, definições");
    CHECA(compila(prog), "ordem compila no gcc");
    ast_free(prog);

    /* Negativos */
    CHECA(falha_com(func("int", NULL, NULL, NULL)), "função sem nome falha");
    CHECA(falha_com(func("Pessoa", "f", NULL, NULL)),
          "tipo de retorno desconhecido falha");
    CHECA(falha_com(func("int", "f", ast_create_param(S("Pessoa"), S("p"),
                                                      0, 1), NULL)),
          "parâmetro de tipo desconhecido falha");
    CHECA(falha_com(func("int", "f", ast_create_param(NULL, S("p"), 0, 1),
                         NULL)),
          "parâmetro sem tipo falha");
    CHECA(falha_com(func("int", "f", ast_create_param(S("int"), NULL, 0, 1),
                         NULL)),
          "parâmetro sem nome falha");
    CHECA(falha_com(func("int", "f", ast_create_param(S("void"), S("v"), 0, 1),
                         NULL)),
          "parâmetro void falha");
    CHECA(falha_com(func("void", "f", NULL,
                         ast_create_return(lit_int("1"), 1))),
          "void retornando valor falha");
    CHECA(falha_com(func("int", "f", NULL, ast_create_return(NULL, 1))),
          "return sem valor em função int falha");
    CHECA(falha_com(ast_create_func_decl(MOD_NONE, S("int"), S("f"), NULL,
                                         NULL, 1)),
          "função sem corpo falha");
    CHECA(falha_com(ast_append(func("int", "f", NULL, NULL),
                               func("int", "f", NULL, NULL))),
          "função repetida (sobrecarga) falha");
    CHECA(falha_com(ast_append(
              ast_create_var_decl(MOD_NONE, S("int"), S("f"), NULL, 1),
              func("int", "f", NULL, NULL))),
          "campo com mesmo nome de função falha");
    CHECA(falha_com(func("int", "while", NULL, NULL)),
          "função com nome reservado falha");
}

static void testes_main(void)
{
    char buf[4096];
    ASTNode *prog;
    ASTNode *args;

    /* public static void main(String[] args) { int a = 1; return; } */
    args = ast_create_param(S("String"), S("args"), 1, 1);
    prog = programa(ast_create_func_decl(MOD_PUBLIC | MOD_STATIC, S("void"),
        S("main"), args, ast_create_block(ast_append(
            ast_create_var_decl(MOD_NONE, S("int"), S("a"), lit_int("1"), 1),
            ast_create_return(NULL, 1)), 1), 1));
    CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "main Java ok");
    CHECA(strstr(buf, "int main(void)\n{\n") != NULL, "main vira int main(void)");
    CHECA(strstr(buf, "String") == NULL && strstr(buf, "args") == NULL,
          "args não vaza para o C");
    CHECA(strstr(buf, "    return 0;\n    return 0;\n}") != NULL,
          "return; na main vira return 0; e fecha com return 0;");
    CHECA(strstr(buf, "static") == NULL, "static não é gerado");
    CHECA(strstr(buf, "main(void);") == NULL, "main não tem protótipo");
    {
        /* Só uma main: a sintética não pode duplicar */
        const char *p = strstr(buf, "int main(void)");
        CHECA(p != NULL && strstr(p + 1, "int main(void)") == NULL,
              "apenas uma main no C");
    }
    CHECA(compila(prog), "main compila no gcc");
    ast_free(prog);

    /* main com assinatura diferente */
    CHECA(falha_com(func("void", "main", ast_create_param(S("int"), S("x"),
                                                          0, 1), NULL)),
          "main(int) falha");
    CHECA(falha_com(func("int", "main", NULL, NULL)), "int main() falha");
    CHECA(falha_com(ast_append(
              func("void", "main",
                   ast_create_param(S("String"), S("a"), 1, 1), NULL),
              func("void", "main",
                   ast_create_param(S("String"), S("b"), 1, 1), NULL))),
          "duas mains falham");
    CHECA(falha_com(func("void", "main",
              ast_create_param(S("String"), S("a"), 1, 1),
              ast_create_return(lit_int("1"), 1))),
          "main retornando valor falha");
}

static void testes_falha_sem_arquivo(void)
{
    ASTNode *prog;

    /* Classe sem nome falha e não deixa arquivo parcial */
    prog = ast_create_program(ast_create_class(MOD_NONE, NULL, NULL, 3), 1);
    CHECA(codegen_gerar_arquivo(prog, "teste_falha.c") == CODEGEN_ERRO,
          "classe sem nome retorna erro");
    CHECA(!existe("teste_falha.c"), "falha não deixa arquivo final");
    CHECA(!existe("teste_falha.c.tmp"), "falha não deixa .tmp");
    ast_free(prog);

    /* Falha no meio da geração (tipo inválido) também não deixa nada */
    prog = programa(ast_create_var_decl(MOD_NONE, S("Pessoa"), S("p"),
                                        NULL, 2));
    CHECA(codegen_gerar_arquivo(prog, "teste_falha2.c") == CODEGEN_ERRO,
          "erro no meio da geração retorna erro");
    CHECA(!existe("teste_falha2.c") && !existe("teste_falha2.c.tmp"),
          "erro no meio não deixa arquivo parcial");
    ast_free(prog);
}

int main(void)
{
    testes_tipos();
    testes_basicos();
    testes_variaveis();
    testes_funcoes();
    testes_main();
    testes_falha_sem_arquivo();

    printf(falhas ? "\n%d falha(s)\n" : "\ntodos os testes passaram\n", falhas);
    return falhas ? 1 : 0;
}
