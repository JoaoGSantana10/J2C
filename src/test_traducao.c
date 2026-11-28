#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "traducao.h"
#include "ast.h"

static ASTNode* make_int(const char *val) {
    return ast_create_literal(NODE_LITERAL_INT, strdup(val), 1);
}

static ASTNode* make_str(const char *val) {
    return ast_create_literal(NODE_LITERAL_STRING, strdup(val), 1);
}

static ASTNode* make_id(const char *name) {
    return ast_create_identifier(strdup(name), 1);
}

static void capturar_traducao(const ASTNode *node, char *buffer, size_t max_len) {
    FILE *tmp = tmpfile();
    assert(tmp != NULL);
    traduzir_comando(tmp, node, 0);
    rewind(tmp);
    size_t n = fread(buffer, 1, max_len - 1, tmp);
    buffer[n] = '\0';
    fclose(tmp);
}

static void test_bloco_e_declaracao(void) {
    ASTNode *decl = ast_create_var_decl(MOD_NONE, strdup("int"), strdup("x"), make_int("10"), 1);
    ASTNode *block = ast_create_block(decl, 1);

    char buf[512];
    capturar_traducao(block, buf, sizeof(buf));

    assert(strstr(buf, "{\n") != NULL);
    assert(strstr(buf, "int x = 10;\n") != NULL);
    assert(strstr(buf, "}\n") != NULL);

    ast_free(block);
    printf("  [OK] Bloco e declaracao de variavel\n");
}

static void test_atribuicoes(void) {
    ASTNode *a1 = ast_create_assign(AST_ASG_ASSIGN, strdup("a"), make_int("5"), 1);
    ASTNode *a2 = ast_create_assign(AST_ASG_PLUS_ASSIGN, strdup("b"), make_int("2"), 1);
    ASTNode *a3 = ast_create_assign(AST_ASG_MINUS_ASSIGN, strdup("c"), make_int("1"), 1);

    char buf[256];
    capturar_traducao(a1, buf, sizeof(buf));
    assert(strcmp(buf, "a = 5;\n") == 0);

    capturar_traducao(a2, buf, sizeof(buf));
    assert(strcmp(buf, "b += 2;\n") == 0);

    capturar_traducao(a3, buf, sizeof(buf));
    assert(strcmp(buf, "c -= 1;\n") == 0);

    ast_free(a1);
    ast_free(a2);
    ast_free(a3);
    printf("  [OK] Atribuicoes (=, +=, -=)\n");
}

static void test_expressoes_como_comandos(void) {
    ASTNode *call_arg = make_str("\"Ola mundo\"");
    ASTNode *call = ast_create_call(strdup("System.out.println"), call_arg, 1);
    ASTNode *stmt = ast_create_expr_stmt(call, 1);

    char buf[256];
    capturar_traducao(stmt, buf, sizeof(buf));
    assert(strstr(buf, "printf(\"%s\\n\", \"Ola mundo\");\n") != NULL);

    ast_free(stmt);
    printf("  [OK] Expressao como comando (System.out.println)\n");
}

static void test_if_else(void) {
    ASTNode *cond = ast_create_bin_op(AST_OP_GT, make_id("x"), make_int("0"), 1);
    ASTNode *then_stmt = ast_create_assign(AST_ASG_ASSIGN, strdup("y"), make_int("1"), 1);
    ASTNode *then_block = ast_create_block(then_stmt, 1);

    ASTNode *else_stmt = ast_create_assign(AST_ASG_ASSIGN, strdup("y"), make_int("2"), 1);
    ASTNode *else_block = ast_create_block(else_stmt, 1);

    ASTNode *if_node = ast_create_if(cond, then_block, else_block, 1);

    char buf[512];
    capturar_traducao(if_node, buf, sizeof(buf));

    assert(strstr(buf, "if ((x > 0)) {") != NULL);
    assert(strstr(buf, "y = 1;") != NULL);
    assert(strstr(buf, "} else {") != NULL);
    assert(strstr(buf, "y = 2;") != NULL);

    ast_free(if_node);
    printf("  [OK] Estrutura condicional if/else\n");
}

static void test_while_e_do_while(void) {
    ASTNode *cond_w = ast_create_bin_op(AST_OP_LT, make_id("i"), make_int("5"), 1);
    ASTNode *inc = ast_create_unary_op(AST_UN_INC, 1, make_id("i"), 1);
    ASTNode *body_w = ast_create_block(ast_create_expr_stmt(inc, 1), 1);
    ASTNode *while_node = ast_create_while(cond_w, body_w, 1);

    char buf[512];
    capturar_traducao(while_node, buf, sizeof(buf));
    assert(strstr(buf, "while ((i < 5)) {") != NULL);
    assert(strstr(buf, "i++;") != NULL);
    ast_free(while_node);

    ASTNode *cond_dw = ast_create_bin_op(AST_OP_GT, make_id("i"), make_int("0"), 1);
    ASTNode *dec = ast_create_unary_op(AST_UN_DEC, 1, make_id("i"), 1);
    ASTNode *body_dw = ast_create_block(ast_create_expr_stmt(dec, 1), 1);
    ASTNode *do_while_node = ast_create_do_while(body_dw, cond_dw, 1);

    capturar_traducao(do_while_node, buf, sizeof(buf));
    assert(strstr(buf, "do {") != NULL);
    assert(strstr(buf, "i--;") != NULL);
    assert(strstr(buf, "} while ((i > 0));") != NULL);
    ast_free(do_while_node);

    printf("  [OK] Lacos while e do/while\n");
}

static void test_for(void) {
    ASTNode *init = ast_create_var_decl(MOD_NONE, strdup("int"), strdup("i"), make_int("0"), 1);
    ASTNode *cond = ast_create_bin_op(AST_OP_LT, make_id("i"), make_int("10"), 1);
    ASTNode *inc = ast_create_unary_op(AST_UN_INC, 1, make_id("i"), 1);
    ASTNode *asg = ast_create_assign(AST_ASG_PLUS_ASSIGN, strdup("total"), make_id("i"), 1);
    ASTNode *body = ast_create_block(asg, 1);

    ASTNode *for_node = ast_create_for(init, cond, inc, body, 1);

    char buf[512];
    capturar_traducao(for_node, buf, sizeof(buf));

    assert(strstr(buf, "for (int i = 0; (i < 10); i++) {") != NULL);
    assert(strstr(buf, "total += i;") != NULL);

    ast_free(for_node);
    printf("  [OK] Laco for com inicializacao C99\n");
}

static void test_switch_case_default(void) {
    ASTNode *c1_stmt = ast_create_assign(AST_ASG_ASSIGN, strdup("r"), make_int("10"), 1);
    c1_stmt = ast_append(c1_stmt, ast_create_break(1));
    ASTNode *c1 = ast_create_case(make_int("1"), c1_stmt, 1);

    ASTNode *def_stmt = ast_create_assign(AST_ASG_ASSIGN, strdup("r"), make_int("0"), 1);
    def_stmt = ast_append(def_stmt, ast_create_break(1));
    ASTNode *cdef = ast_create_default(def_stmt, 1);

    ASTNode *cases = ast_append(c1, cdef);
    ASTNode *sw = ast_create_switch(make_id("x"), cases, 1);

    char buf[512];
    capturar_traducao(sw, buf, sizeof(buf));

    assert(strstr(buf, "switch (x) {") != NULL);
    assert(strstr(buf, "case 1:\n") != NULL);
    assert(strstr(buf, "r = 10;\n") != NULL);
    assert(strstr(buf, "break;\n") != NULL);
    assert(strstr(buf, "default:\n") != NULL);
    assert(strstr(buf, "r = 0;\n") != NULL);

    ast_free(sw);
    printf("  [OK] Switch, case, default e break\n");
}

static void test_jumps_e_return(void) {
    ASTNode *brk = ast_create_break(1);
    ASTNode *cnt = ast_create_continue(1);
    ASTNode *ret_vazio = ast_create_return(NULL, 1);
    ASTNode *ret_val = ast_create_return(make_int("42"), 1);

    char buf[256];
    capturar_traducao(brk, buf, sizeof(buf));
    assert(strcmp(buf, "break;\n") == 0);

    capturar_traducao(cnt, buf, sizeof(buf));
    assert(strcmp(buf, "continue;\n") == 0);

    capturar_traducao(ret_vazio, buf, sizeof(buf));
    assert(strcmp(buf, "return;\n") == 0);

    capturar_traducao(ret_val, buf, sizeof(buf));
    assert(strcmp(buf, "return 42;\n") == 0);

    ast_free(brk);
    ast_free(cnt);
    ast_free(ret_vazio);
    ast_free(ret_val);
    printf("  [OK] Comandos break, continue e return\n");
}

static void test_compilacao_c_gerado(void) {
    FILE *f = fopen("tmp_validacao_traducao.c", "w");
    assert(f != NULL);

    fprintf(f, "#include <stdio.h>\n#include <stdbool.h>\n\n");
    fprintf(f, "int test_full_flow(int val) {\n");

    ASTNode *d_total = ast_create_var_decl(MOD_NONE, strdup("int"), strdup("total"), make_int("0"), 1);
    traduzir_comando(f, d_total, 1);

    ASTNode *init_for = ast_create_var_decl(MOD_NONE, strdup("int"), strdup("i"), make_int("0"), 1);
    ASTNode *cond_for = ast_create_bin_op(AST_OP_LT, make_id("i"), make_int("10"), 1);
    ASTNode *inc_for = ast_create_unary_op(AST_UN_INC, 1, make_id("i"), 1);

    ASTNode *if_cond = ast_create_bin_op(AST_OP_EQ, make_id("i"), make_int("5"), 1);
    ASTNode *if_body = ast_create_block(ast_create_continue(1), 1);
    ASTNode *if_node = ast_create_if(if_cond, if_body, NULL, 1);

    ASTNode *add_stmt = ast_create_assign(AST_ASG_PLUS_ASSIGN, strdup("total"), make_id("i"), 1);
    ASTNode *for_body_stmts = ast_append(if_node, add_stmt);
    ASTNode *for_node = ast_create_for(init_for, cond_for, inc_for, ast_create_block(for_body_stmts, 1), 1);
    traduzir_comando(f, for_node, 1);

    ASTNode *ret_node = ast_create_return(ast_create_bin_op(AST_OP_ADD, make_id("total"), make_id("val"), 1), 1);
    traduzir_comando(f, ret_node, 1);

    fprintf(f, "}\n\nint main(void) {\n    return (test_full_flow(10) > 0) ? 0 : 1;\n}\n");
    fclose(f);

    int status = system("gcc -Wall -Wextra -Werror tmp_validacao_traducao.c -o tmp_validacao_bin && ./tmp_validacao_bin");
    remove("tmp_validacao_traducao.c");
    remove("tmp_validacao_bin");

    assert(status == 0);
    printf("  [OK] Validacao de compilacao e execucao do codigo C gerado\n");

    ast_free(d_total);
    ast_free(for_node);
    ast_free(ret_node);
}

int main(void) {
    printf("Iniciando testes unitarios de traducao da AST...\n");
    test_bloco_e_declaracao();
    test_atribuicoes();
    test_expressoes_como_comandos();
    test_if_else();
    test_while_e_do_while();
    test_for();
    test_switch_case_default();
    test_jumps_e_return();
    test_compilacao_c_gerado();
    printf("Todos os testes de traducao passaram com sucesso!\n");
    return 0;
}
