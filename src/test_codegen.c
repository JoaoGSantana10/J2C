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

/* ---- casos que devem gerar `trecho`: AST montada com `membros` ------ */
static int gera_contem(ASTNode *membros, const char *trecho)
{
    char buf[8192];
    ASTNode *prog = programa(membros);
    int ok = gera(prog, buf, sizeof buf) == CODEGEN_OK &&
             strstr(buf, trecho) != NULL;

    if (!ok)
        printf("        esperado \"%s\" em:\n%s\n", trecho, buf);
    ast_free(prog);
    return ok;
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
    CHECA(strstr(buf, "long long a = 5LL;") != NULL, "long long (5L -> 5LL)");
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
    CHECA(gera_contem(ast_create_var_decl(MOD_NONE, S("int"), S("x"),
              ast_create_bin_op(AST_OP_ADD, lit_int("1"), lit_int("2"), 2), 2),
              "int x = 1 + 2;\n"),
          "campo com expressão constante (1 + 2) é aceito");
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

/* ------------------------- expressões ------------------------------ */

static ASTNode *id(const char *n) { return ast_create_identifier(S(n), 1); }
static ASTNode *lit(NodeType t, const char *v)
{
    return ast_create_literal(t, S(v), 1);
}
static ASTNode *bin(BinaryOpType op, ASTNode *l, ASTNode *r)
{
    return ast_create_bin_op(op, l, r, 1);
}
static ASTNode *un(UnaryOpType op, int posfixo, ASTNode *o)
{
    return ast_create_unary_op(op, posfixo, o, 1);
}
static ASTNode *pint(const char *n) { return ast_create_param(S("int"), S(n), 0, 1); }
static ASTNode *decl(const char *tipo, const char *n, ASTNode *init)
{
    return ast_create_var_decl(MOD_NONE, S(tipo), S(n), init, 1);
}
static ASTNode *cmd(ASTNode *e) { return ast_create_expr_stmt(e, 1); }

/* int f(int a, int b, int c, int i, int j) { <stmts> return 0; } */
static ASTNode *func_abc(ASTNode *stmts)
{
    return func("int", "f",
        ast_append(ast_append(ast_append(ast_append(pint("a"), pint("b")),
                   pint("c")), pint("i")), pint("j")),
        ast_append(stmts, ast_create_return(lit_int("0"), 1)));
}

/* Retorna o membro f(...) com `int r = <e>;` no corpo, gerado com sucesso
 * e contendo `trecho`. */
static int expr_gera(ASTNode *e, const char *trecho)
{
    return gera_contem(func_abc(decl("int", "r", e)), trecho);
}

static int expr_falha(ASTNode *e)
{
    return falha_com(func_abc(decl("int", "r", e)));
}

static void testes_expressoes(void)
{
    ASTNode *prog;

    /* Exemplo positivo da issue:
     *   int exemplo(int a, int b, int c, int x, int y) {
     *       int resultado = a + b * c;  resultado += 2;  resultado++;
     *       calcular(x, y + 1);  return resultado;
     *   }
     *   void calcular(int p, int q) { } */
    prog = programa(ast_append(
        func("int", "exemplo",
            ast_append(ast_append(ast_append(ast_append(pint("a"), pint("b")),
                       pint("c")), pint("x")), pint("y")),
            ast_append(ast_append(ast_append(ast_append(
                decl("int", "resultado",
                     bin(AST_OP_ADD, id("a"), bin(AST_OP_MUL, id("b"), id("c")))),
                ast_create_assign(AST_ASG_PLUS_ASSIGN, S("resultado"),
                                  lit_int("2"), 1)),
                cmd(un(AST_UN_INC, 1, id("resultado")))),
                cmd(ast_create_call(S("calcular"),
                    ast_append(id("x"), bin(AST_OP_ADD, id("y"), lit_int("1"))),
                    1))),
                ast_create_return(id("resultado"), 1))),
        func("void", "calcular", ast_append(pint("p"), pint("q")), NULL)));
    {
        char buf[8192];

        CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "exemplo da issue ok");
        CHECA(strstr(buf, "    int resultado = a + (b * c);\n") != NULL,
              "a + b * c -> a + (b * c)");
        CHECA(strstr(buf, "    resultado += 2;\n") != NULL, "resultado += 2;");
        CHECA(strstr(buf, "    resultado++;\n") != NULL, "resultado++;");
        CHECA(strstr(buf, "    calcular(x, y + 1);\n") != NULL,
              "calcular(x, y + 1);");
    }
    CHECA(compila(prog), "exemplo da issue compila no gcc");
    ast_free(prog);

    /* Precedência e associatividade: os parênteses seguem a AST */
    CHECA(expr_gera(bin(AST_OP_MUL, bin(AST_OP_ADD, id("a"), id("b")), id("c")),
                    "int r = (a + b) * c;"), "(a + b) * c mantém os parênteses");
    CHECA(expr_gera(bin(AST_OP_SUB, bin(AST_OP_SUB, id("a"), id("b")), id("c")),
                    "int r = (a - b) - c;"), "a - b - c associa à esquerda");
    CHECA(expr_gera(bin(AST_OP_SUB, id("a"), bin(AST_OP_SUB, id("b"), id("c"))),
                    "int r = a - (b - c);"), "a - (b - c) mantém o agrupamento");
    CHECA(expr_gera(bin(AST_OP_DIV, bin(AST_OP_DIV, id("a"), id("b")), id("c")),
                    "int r = (a / b) / c;"), "a / b / c associa à esquerda");
    CHECA(expr_gera(bin(AST_OP_OR,
                        bin(AST_OP_AND, bin(AST_OP_LT, id("a"), id("b")),
                                        bin(AST_OP_EQ, id("c"), id("i"))),
                        un(AST_UN_NOT, 0, id("j"))),
                    "int r = ((a < b) && (c == i)) || !j;"),
          "lógicos e relacionais preservam a precedência");

    /* Unários: prefixo x pósfixo */
    CHECA(gera_contem(func_abc(ast_append(ast_append(ast_append(
              cmd(un(AST_UN_INC, 0, id("i"))), cmd(un(AST_UN_INC, 1, id("i")))),
              cmd(un(AST_UN_DEC, 0, id("j")))), cmd(un(AST_UN_DEC, 1, id("j"))))),
              "    ++i;\n    i++;\n    --j;\n    j--;\n"),
          "++i, i++, --j e j-- (is_postfix respeitado)");
    CHECA(expr_gera(bin(AST_OP_ADD, un(AST_UN_INC, 1, id("i")),
                                    un(AST_UN_DEC, 0, id("j"))),
                    "int r = i++ + --j;"), "i++ + --j");
    CHECA(expr_gera(bin(AST_OP_MUL, un(AST_UN_NEG, 0, id("a")), id("b")),
                    "int r = -a * b;"), "-a * b");
    CHECA(expr_gera(un(AST_UN_NEG, 0, bin(AST_OP_ADD, id("a"), id("b"))),
                    "int r = -(a + b);"), "-(a + b)");
    CHECA(expr_gera(un(AST_UN_NEG, 0, un(AST_UN_NEG, 0, id("a"))),
                    "int r = -(-a);"), "-(-a) não vira --a");

    /* Literais adaptados ao C */
    prog = programa(func("void", "literais", NULL, ast_append(ast_append(
        ast_append(ast_append(ast_append(ast_append(ast_append(ast_append(
            decl("long", "l", lit(NODE_LITERAL_LONG, "10L")),
            decl("double", "d1", lit(NODE_LITERAL_DOUBLE, "3.14d"))),
            decl("double", "d2", lit(NODE_LITERAL_DOUBLE, "5d"))),
            decl("float", "f1", lit(NODE_LITERAL_FLOAT, "5f"))),
            decl("float", "f2", lit(NODE_LITERAL_FLOAT, "2.5F"))),
            decl("char", "c1", lit(NODE_LITERAL_CHAR, "'\\'"))),
            decl("char", "c2", lit(NODE_LITERAL_CHAR, "'''"))),
            decl("boolean", "b", lit(NODE_LITERAL_BOOL, "false"))),
            decl("String", "s", lit(NODE_LITERAL_STRING,
                                    "\"a\\n\\u0041\\400?\?=\\\"")))));
    {
        char buf[8192];

        CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK, "literais ok");
        CHECA(strstr(buf, "long long l = 10LL;") != NULL, "10L -> 10LL");
        CHECA(strstr(buf, "double d1 = 3.14;") != NULL, "3.14d -> 3.14");
        CHECA(strstr(buf, "double d2 = 5.0;") != NULL, "5d -> 5.0");
        CHECA(strstr(buf, "float f1 = 5.0f;") != NULL, "5f -> 5.0f");
        CHECA(strstr(buf, "float f2 = 2.5f;") != NULL, "2.5F -> 2.5f");
        CHECA(strstr(buf, "char c1 = '\\\\';") != NULL, "'\\' -> '\\\\'");
        CHECA(strstr(buf, "char c2 = '\\'';") != NULL, "''' -> '\\''");
        CHECA(strstr(buf, "bool b = false;") != NULL, "false -> false");
        /* \n mantido, \u0041 -> \101, \400 (Java: \40 + '0') -> \0400,
         * ??= sem trígrafo, barra final escapada */
        CHECA(strstr(buf, "char *s = \"a\\n\\101\\0400?\\?=\\\\\";") != NULL,
              "String: \\n, \\u, octal, trígrafo e barra final");
    }
    CHECA(compila(prog), "literais compilam no gcc");
    ast_free(prog);

    /* Campos aceitam expressões constantes */
    CHECA(gera_contem(decl("int", "k", un(AST_UN_NEG, 0, lit_int("1"))),
                      "int k = -1;\n"), "campo int k = -1 é aceito");
    CHECA(falha_com(decl("int", "k", ast_create_call(S("g"), NULL, 1))),
          "campo inicializado com chamada falha");

    /* --- casos negativos da issue --- */
    CHECA(expr_falha(bin((BinaryOpType)99, id("a"), id("b"))),
          "operador binário desconhecido falha");
    CHECA(expr_falha(un((UnaryOpType)99, 0, id("a"))),
          "operador unário desconhecido falha");
    CHECA(expr_falha(ast_create_literal(NODE_LITERAL_INT, NULL, 1)),
          "literal sem valor falha");
    CHECA(expr_falha(ast_create_identifier(S(""), 1)),
          "identificador vazio falha");
    CHECA(expr_falha(ast_create_call(S(""), NULL, 1)), "chamada sem nome falha");
    CHECA(expr_falha(un(AST_UN_NEG, 0, NULL)), "unário sem operando falha");
    CHECA(expr_falha(bin(AST_OP_ADD, id("a"), NULL)),
          "binário com apenas um filho falha");
    CHECA(expr_falha(ast_create_block(NULL, 1)),
          "expressão mal formada (bloco como valor) falha");
    CHECA(expr_falha(ast_create_new(NULL, NULL, 1)), "new sem classe falha");
    CHECA(expr_falha(ast_create_new(S("Scanner"), S("System.in"), 1)),
          "new Scanner(System.in) é rejeitado (issue de E/S)");
    CHECA(falha_com(func_abc(cmd(ast_create_call(S("Math.max"),
              ast_append(id("a"), id("b")), 1)))),
          "chamada qualificada não é descartada: falha");
    CHECA(expr_falha(un(AST_UN_INC, 1, lit_int("5"))), "5++ falha");

    /* Literais inválidos */
    CHECA(expr_falha(lit(NODE_LITERAL_INT, "09")), "literal octal 09 falha");
    CHECA(expr_falha(lit(NODE_LITERAL_CHAR, "'ab'")), "char 'ab' falha");
    CHECA(expr_falha(lit(NODE_LITERAL_BOOL, "verdadeiro")),
          "boolean inválido falha");
    CHECA(falha_com(func_abc(decl("String", "s",
              lit(NODE_LITERAL_STRING, "\"\\q\"")))),
          "escape \\q inválido falha");
    CHECA(falha_com(func_abc(decl("String", "s",
              lit(NODE_LITERAL_STRING, "\"\\u00\"")))),
          "\\u incompleto falha");

    /* Chamadas */
    CHECA(falha_com(ast_append(func_abc(cmd(ast_create_call(S("g"),
              id("a"), 1))), func("void", "g", ast_append(pint("p"), pint("q")),
              NULL))),
          "chamada com argumentos a menos falha");
    CHECA(falha_com(ast_append(func_abc(decl("int", "r",
              ast_create_call(S("g"), NULL, 1))), func("void", "g", NULL, NULL))),
          "função void usada como valor falha");
    CHECA(falha_com(func_abc(cmd(ast_create_call(S("h"), NULL, 1)))),
          "função não declarada falha");
    CHECA(expr_falha(id("f")), "nome de função usado como valor falha");
    CHECA(falha_com(func_abc(ast_create_assign(AST_ASG_ASSIGN, S("f"),
              lit_int("1"), 1))), "atribuição a função falha");
    CHECA(falha_com(ast_append(func("int", "g", NULL, ast_append(
              decl("int", "h", lit_int("1")),
              ast_create_return(ast_create_call(S("h"), NULL, 1), 1))),
              func("int", "h", NULL, ast_create_return(lit_int("1"), 1)))),
          "variável local que esconde função falha na chamada");
    CHECA(falha_com(func_abc(cmd(id("a")))), "comando sem efeito (a;) falha");

    /* Traduções que mudariam o significado */
    CHECA(falha_com(func_abc(decl("String", "s",
              bin(AST_OP_ADD, lit(NODE_LITERAL_STRING, "\"n=\""), id("a"))))),
          "concatenação de String com + falha");
    CHECA(falha_com(func_abc(ast_append(decl("String", "s",
              lit(NODE_LITERAL_STRING, "\"x\"")),
              ast_create_assign(AST_ASG_PLUS_ASSIGN, S("s"),
                                lit(NODE_LITERAL_STRING, "\"y\""), 1)))),
          "String += falha");
    CHECA(expr_falha(bin(AST_OP_MOD, lit(NODE_LITERAL_DOUBLE, "5.5"),
                         lit_int("2"))), "% com double falha");
    CHECA(expr_falha(bin(AST_OP_ADD, un(AST_UN_INC, 1, id("i")), id("i"))),
          "i++ + i (indefinido em C) falha");
    CHECA(falha_com(func_abc(ast_create_assign(AST_ASG_ASSIGN, S("i"),
              un(AST_UN_INC, 1, id("i")), 1))), "i = i++ falha");
    CHECA(expr_gera(bin(AST_OP_ADD, un(AST_UN_INC, 1, id("i")), id("j")),
                    "int r = i++ + j;"), "i++ + j (variáveis diferentes) é aceito");
}

/* ------------------------ entrada e saída -------------------------- */

static ASTNode *blk(ASTNode *stmts) { return ast_create_block(stmts, 1); }

static ASTNode *saida(const char *nome, ASTNode *arg)
{
    return cmd(ast_create_call(S(nome), arg, 1));
}
static ASTNode *scanner_decl(const char *nome)
{
    return decl("Scanner", nome, ast_create_new(S("Scanner"), S("System.in"), 1));
}

/* void main(String[] args) { <stmts> } */
static ASTNode *main_com(ASTNode *stmts)
{
    return ast_create_func_decl(MOD_PUBLIC | MOD_STATIC, S("void"), S("main"),
        ast_create_param(S("String"), S("args"), 1, 1), blk(stmts), 1);
}

static int io_gera(ASTNode *membros, const char *trecho)
{
    char buf[8192];
    ASTNode *prog = programa(membros);
    int ok = gera(prog, buf, sizeof buf) == CODEGEN_OK &&
             strstr(buf, trecho) != NULL;

    if (!ok)
        printf("        esperado \"%s\" em:\n%s\n", trecho, buf);
    ok = ok && compila(prog);
    ast_free(prog);
    return ok;
}

static void testes_entrada_saida(void)
{
    /* Formato escolhido pelo tipo do argumento */
    CHECA(io_gera(main_com(saida("System.out.println",
              lit(NODE_LITERAL_STRING, "\"Olá\""))),
              "    printf(\"%s\\n\", \"Ol"), "println(String literal) -> %s\\n");
    CHECA(io_gera(main_com(ast_append(decl("int", "n", lit_int("10")),
              saida("System.out.println", id("n")))),
              "    printf(\"%d\\n\", n);\n"), "println(int) -> %d");
    CHECA(io_gera(main_com(ast_append(decl("int", "n", lit_int("10")),
              saida("System.out.println", bin(AST_OP_ADD, id("n"), lit_int("1"))))),
              "    printf(\"%d\\n\", n + 1);\n"), "println(n + 1) -> %d");
    CHECA(io_gera(main_com(saida("System.out.println",
              lit(NODE_LITERAL_LONG, "5L"))),
              "printf(\"%lld\\n\", 5LL);"), "println(long) -> %lld");
    CHECA(io_gera(main_com(saida("System.out.println",
              lit(NODE_LITERAL_DOUBLE, "2.5"))),
              "printf(\"%lf\\n\", 2.5);"), "println(double) -> %lf");
    CHECA(io_gera(main_com(saida("System.out.println",
              lit(NODE_LITERAL_FLOAT, "1.5f"))),
              "printf(\"%f\\n\", 1.5f);"), "println(float) -> %f");
    CHECA(io_gera(main_com(saida("System.out.println",
              lit(NODE_LITERAL_CHAR, "'x'"))),
              "printf(\"%c\\n\", 'x');"), "println(char) -> %c");
    CHECA(io_gera(main_com(saida("System.out.println",
              bin(AST_OP_LT, lit_int("1"), lit_int("2")))),
              "printf(\"%s\\n\", (1 < 2) ? \"true\" : \"false\");"),
          "println(boolean) -> true/false");
    CHECA(io_gera(main_com(saida("System.out.print",
              lit(NODE_LITERAL_STRING, "\"x\""))),
              "printf(\"%s\", \"x\");"), "print sem quebra de linha");
    CHECA(io_gera(main_com(saida("System.out.println", NULL)),
              "    printf(\"\\n\");\n"), "println() -> printf(\"\\n\")");

    /* Scanner: declaração vira comentário, nextInt/nextDouble viram as
     * funções auxiliares, close não faz nada */
    CHECA(io_gera(main_com(ast_append(ast_append(ast_append(scanner_decl("sc"),
              decl("int", "v", ast_create_call(S("sc.nextInt"), NULL, 1))),
              decl("double", "d", ast_create_call(S("sc.nextDouble"), NULL, 1))),
              cmd(ast_create_call(S("sc.close"), NULL, 1)))),
              "    /* Scanner sc: leitura da entrada padrão */\n"
              "    int v = j2c_ler_int();\n"
              "    double d = j2c_ler_double();\n"
              "    (void)0;\n"),
          "Scanner, nextInt, nextDouble e close");
    CHECA(io_gera(main_com(scanner_decl("sc")), "int j2c_ler_int(void)\n{"),
          "funções auxiliares geradas quando há Scanner");
    CHECA(io_gera(ast_append(scanner_decl("entrada"),
              main_com(saida("System.out.println",
                  ast_create_call(S("entrada.nextInt"), NULL, 1)))),
              "printf(\"%d\\n\", j2c_ler_int());"),
          "Scanner como campo e println(sc.nextInt())");
    {
        char buf[4096];
        ASTNode *prog = programa(main_com(saida("System.out.println", NULL)));

        CHECA(gera(prog, buf, sizeof buf) == CODEGEN_OK &&
              strstr(buf, "j2c_ler_int") == NULL,
              "sem Scanner, sem funções auxiliares");
        ast_free(prog);
    }

    /* --- negativos --- */
    CHECA(falha_com(main_com(saida("System.out.println",
              ast_append(lit_int("1"), lit_int("2"))))),
          "println com dois argumentos falha");
    CHECA(falha_com(main_com(saida("System.out.print", NULL))),
          "print sem argumento falha");
    CHECA(falha_com(main_com(saida("System.out.println",
              bin(AST_OP_ADD, lit(NODE_LITERAL_STRING, "\"n=\""), lit_int("1"))))),
          "println com concatenação falha");
    CHECA(falha_com(ast_append(main_com(saida("System.out.println",
              ast_create_call(S("g"), NULL, 1))), func("void", "g", NULL, NULL))),
          "println de função void falha");
    CHECA(falha_com(main_com(decl("int", "x", ast_create_call(
              S("System.out.println"), NULL, 1)))),
          "println usado como valor falha");
    CHECA(falha_com(main_com(ast_append(scanner_decl("sc"),
              decl("String", "s", ast_create_call(S("sc.nextLine"), NULL, 1))))),
          "Scanner.nextLine (não suportado) falha");
    CHECA(falha_com(main_com(decl("Scanner", "sc",
              ast_create_new(S("Scanner"), S("arquivo"), 1)))),
          "Scanner sem System.in falha");
    CHECA(falha_com(main_com(decl("Scanner", "sc", NULL))),
          "Scanner sem new falha");
    CHECA(falha_com(main_com(ast_append(scanner_decl("sc"),
              saida("System.out.println", id("sc"))))),
          "Scanner usado como valor falha");
    CHECA(falha_com(main_com(ast_append(scanner_decl("sc"),
              decl("int", "v", ast_create_call(S("sc.nextInt"), lit_int("1"), 1))))),
          "nextInt com argumento falha");
}

/* ------------------------ controle de fluxo ------------------------ */

static ASTNode *asg(AssignOpType op, const char *v, ASTNode *e)
{
    return ast_create_assign(op, S(v), e, 1);
}
static ASTNode *caso(const char *v, ASTNode *stmts)
{
    return ast_create_case(lit_int(v), stmts, 1);
}

/* int f(int a, int b, int c, int i, int j) com `stmts` no corpo: gera,
 * confere o trecho e compila no gcc. */
static int controle_gera(ASTNode *stmts, const char *trecho)
{
    return io_gera(func_abc(stmts), trecho);
}

static void testes_controle(void)
{
    /* if / else if / else, com e sem bloco no Java */
    CHECA(controle_gera(
              ast_create_if(bin(AST_OP_GT, id("a"), lit_int("0")),
                  blk(asg(AST_ASG_ASSIGN, "b", lit_int("1"))),
                  ast_create_if(bin(AST_OP_LT, id("a"), lit_int("0")),
                      asg(AST_ASG_ASSIGN, "b", lit_int("2")),
                      asg(AST_ASG_ASSIGN, "b", lit_int("3")), 1), 1),
              "    if (a > 0)\n"
              "    {\n"
              "        b = 1;\n"
              "    }\n"
              "    else if (a < 0)\n"
              "    {\n"
              "        b = 2;\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        b = 3;\n"
              "    }\n"),
          "if / else if / else (corpo sem bloco ganha chaves)");
    CHECA(controle_gera(
              ast_create_if(bin(AST_OP_EQ, id("a"), id("b")),
                  ast_create_return(lit_int("1"), 1), NULL, 1),
              "    if (a == b)\n    {\n        return 1;\n    }\n    return 0;"),
          "if sem else");
    /* else associado ao if certo: if (a) { if (b) x; } else y; */
    CHECA(controle_gera(
              ast_create_if(bin(AST_OP_GT, id("a"), lit_int("0")),
                  ast_create_if(bin(AST_OP_GT, id("b"), lit_int("0")),
                      asg(AST_ASG_ASSIGN, "c", lit_int("1")), NULL, 1),
                  asg(AST_ASG_ASSIGN, "c", lit_int("2")), 1),
              "    if (a > 0)\n"
              "    {\n"
              "        if (b > 0)\n"
              "        {\n"
              "            c = 1;\n"
              "        }\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        c = 2;\n"
              "    }\n"),
          "else fica com o if externo (if aninhado)");

    /* Declaração como corpo sem bloco: em C só vale dentro de chaves */
    CHECA(controle_gera(
              ast_create_if(bin(AST_OP_GT, id("a"), lit_int("0")),
                  decl("int", "y", lit_int("1")), NULL, 1),
              "    if (a > 0)\n    {\n        int y = 1;\n    }\n"),
          "declaração como corpo do if fica entre chaves");

    /* while e do-while, com break e continue */
    CHECA(controle_gera(
              ast_create_while(bin(AST_OP_LT, id("i"), lit_int("10")),
                  blk(ast_append(ast_append(
                      cmd(un(AST_UN_INC, 1, id("i"))),
                      ast_create_if(bin(AST_OP_EQ, id("i"), lit_int("5")),
                          blk(ast_create_continue(1)), NULL, 1)),
                      ast_create_if(bin(AST_OP_EQ, id("i"), lit_int("8")),
                          blk(ast_create_break(1)), NULL, 1))), 1),
              "    while (i < 10)\n"
              "    {\n"
              "        i++;\n"
              "        if (i == 5)\n"
              "        {\n"
              "            continue;\n"
              "        }\n"
              "        if (i == 8)\n"
              "        {\n"
              "            break;\n"
              "        }\n"
              "    }\n"),
          "while com continue e break");
    CHECA(controle_gera(
              ast_create_do_while(blk(cmd(un(AST_UN_DEC, 1, id("i")))),
                  bin(AST_OP_GT, id("i"), lit_int("0")), 1),
              "    do\n    {\n        i--;\n    }\n    while (i > 0);\n"),
          "do-while");

    /* for: init declarado (C99), atribuição, partes vazias */
    CHECA(controle_gera(
              ast_create_for(decl("int", "k", lit_int("0")),
                  bin(AST_OP_LT, id("k"), lit_int("10")),
                  un(AST_UN_INC, 1, id("k")),
                  blk(asg(AST_ASG_PLUS_ASSIGN, "a", id("k"))), 1),
              "    for (int k = 0; k < 10; k++)\n    {\n        a += k;\n    }\n"),
          "for com declaração no init");
    CHECA(controle_gera(
              ast_create_for(asg(AST_ASG_ASSIGN, "i", lit_int("0")),
                  bin(AST_OP_LT, id("i"), id("a")),
                  asg(AST_ASG_PLUS_ASSIGN, "i", lit_int("2")),
                  cmd(un(AST_UN_INC, 1, id("b"))), 1),
              "    for (i = 0; i < a; i += 2)\n    {\n        b++;\n    }\n"),
          "for com atribuições no init e no incremento");
    CHECA(controle_gera(
              ast_create_for(NULL, NULL, NULL, blk(ast_create_break(1)), 1),
              "    for (;;)\n    {\n        break;\n    }\n"),
          "for (;;) com partes vazias");
    CHECA(controle_gera(
              ast_create_for(NULL, bin(AST_OP_LT, id("i"), lit_int("3")), NULL,
                  blk(cmd(un(AST_UN_INC, 1, id("i")))), 1),
              "    for (; i < 3;)\n"),
          "for (; condição;) preserva as partes vazias");
    /* O 'int k' do init some no fim do for: dois for seguidos podem usar */
    CHECA(controle_gera(ast_append(
              ast_create_for(decl("int", "k", lit_int("0")),
                  bin(AST_OP_LT, id("k"), lit_int("2")),
                  un(AST_UN_INC, 1, id("k")), blk(NULL), 1),
              ast_create_for(decl("double", "k", lit(NODE_LITERAL_DOUBLE, "0.5")),
                  bin(AST_OP_LT, id("k"), lit_int("2")),
                  asg(AST_ASG_PLUS_ASSIGN, "k", lit(NODE_LITERAL_DOUBLE, "1.0")),
                  blk(NULL), 1)),
              "    for (double k = 0.5; k < 2; k += 1.0)\n    {\n    }\n"),
          "escopo do init do for termina com o for");

    /* switch, case, default, fallthrough e break vindos da AST */
    CHECA(controle_gera(
              ast_create_switch(id("a"), ast_append(ast_append(ast_append(
                  caso("1", NULL),
                  caso("2", ast_append(asg(AST_ASG_ASSIGN, "b", lit_int("10")),
                                       ast_create_break(1)))),
                  ast_create_case(lit(NODE_LITERAL_CHAR, "'x'"),
                      ast_create_return(lit_int("7"), 1), 1)),
                  ast_create_default(ast_append(
                      asg(AST_ASG_ASSIGN, "b", lit_int("0")),
                      ast_create_break(1)), 1)), 1),
              "    switch (a)\n"
              "    {\n"
              "        case 1:\n"
              "        case 2:\n"
              "            b = 10;\n"
              "            break;\n"
              "        case 'x':\n"
              "            return 7;\n"
              "        default:\n"
              "            b = 0;\n"
              "            break;\n"
              "    }\n"),
          "switch com case vazio (fallthrough), char e default");
    /* Rótulo antes de declaração e rótulo no fim do switch: ';' vazio */
    CHECA(controle_gera(
              ast_create_switch(id("a"), ast_append(
                  caso("1", ast_append(decl("int", "t", lit_int("3")),
                                       asg(AST_ASG_ASSIGN, "b", id("t")))),
                  ast_create_default(NULL, 1)), 1),
              "        case 1: ;\n"
              "            int t = 3;\n"
              "            b = t;\n"
              "        default: ;\n"
              "    }\n"),
          "case seguido de declaração e default vazio no fim");
    /* continue dentro de switch dentro de laço é válido */
    CHECA(controle_gera(
              ast_create_while(bin(AST_OP_LT, id("i"), lit_int("3")),
                  blk(ast_append(cmd(un(AST_UN_INC, 1, id("i"))),
                      ast_create_switch(id("i"),
                          caso("1", ast_create_continue(1)), 1))), 1),
              "            case 1:\n                continue;\n"),
          "continue em switch dentro de laço");

    /* return 0 na main dentro de laço */
    CHECA(io_gera(main_com(ast_create_while(lit(NODE_LITERAL_BOOL, "true"),
              blk(ast_create_return(NULL, 1)), 1)),
              "    while (true)\n    {\n        return 0;\n    }\n"),
          "return; dentro de laço na main vira return 0;");

    /* --- negativos --- */
    CHECA(falha_com(func_abc(ast_create_break(1))), "break fora de laço falha");
    CHECA(falha_com(func_abc(ast_create_continue(1))),
          "continue fora de laço falha");
    CHECA(falha_com(func_abc(ast_create_switch(id("a"),
              caso("1", ast_create_continue(1)), 1))),
          "continue em switch fora de laço falha");
    CHECA(falha_com(func_abc(ast_create_if(id("a"), blk(NULL), NULL, 1))),
          "if com condição int falha");
    CHECA(falha_com(func_abc(ast_append(decl("String", "s",
              lit(NODE_LITERAL_STRING, "\"x\"")),
              ast_create_while(id("s"), blk(NULL), 1)))),
          "while com condição String falha");
    CHECA(falha_com(func_abc(ast_create_if(NULL, blk(NULL), NULL, 1))),
          "if sem condição falha");
    CHECA(falha_com(func_abc(ast_create_while(NULL, blk(NULL), 1))),
          "while sem condição falha");
    CHECA(falha_com(func_abc(ast_create_switch(NULL, NULL, 1))),
          "switch sem expressão falha");
    CHECA(falha_com(func_abc(ast_create_if(id("a"), NULL, NULL, 1))),
          "if sem corpo falha");
    CHECA(falha_com(func_abc(ast_create_for(NULL, NULL,
              bin(AST_OP_ADD, id("i"), lit_int("1")), blk(NULL), 1))),
          "incremento do for sem efeito (i + 1) falha");
    CHECA(falha_com(func_abc(ast_create_for(cmd(un(AST_UN_INC, 1, id("i"))),
              NULL, NULL, blk(NULL), 1))),
          "init do for que não é declaração nem atribuição falha");
    CHECA(falha_com(func_abc(ast_append(decl("String", "s",
              lit(NODE_LITERAL_STRING, "\"x\"")),
              ast_create_switch(id("s"), NULL, 1)))),
          "switch sobre String falha");
    CHECA(falha_com(func_abc(ast_create_switch(id("a"),
              ast_create_case(lit(NODE_LITERAL_STRING, "\"x\""), NULL, 1), 1))),
          "case com String falha");
    CHECA(falha_com(func_abc(ast_create_switch(id("a"),
              ast_append(caso("1", NULL), caso("1", NULL)), 1))),
          "case repetido falha");
    CHECA(falha_com(func_abc(ast_create_switch(id("a"),
              ast_append(ast_create_default(NULL, 1),
                         ast_create_default(NULL, 1)), 1))),
          "dois defaults falham");
    CHECA(falha_com(func_abc(caso("1", NULL))),
          "case fora de switch (lista de cases no nó errado) falha");
    CHECA(falha_com(func_abc(ast_create_if(
              bin(AST_OP_LT, un(AST_UN_INC, 1, id("i")), id("i")),
              blk(NULL), NULL, 1))),
          "condição com i++ < i (indefinido em C) falha");
    CHECA(falha_com(main_com(ast_create_for(decl("Scanner", "sc",
              ast_create_new(S("Scanner"), S("System.in"), 1)),
              NULL, NULL, blk(NULL), 1))),
          "Scanner declarado no init do for falha");
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
    testes_expressoes();
    testes_entrada_saida();
    testes_controle();
    testes_falha_sem_arquivo();

    printf(falhas ? "\n%d falha(s)\n" : "\ntodos os testes passaram\n", falhas);
    return falhas ? 1 : 0;
}
