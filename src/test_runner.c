#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h" /* Cabeçalho do Bison contendo as constantes dos tokens */

/* Declarações internas do Flex */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yylex(void);
extern int contLinha;

int total_tests = 0;
int passed_tests = 0;

/* Função que executa o scanner e valida o TOKEN retornado pelo yylex() */
void run_test(const char *test_name, const char *input, int expected_token)
{
    total_tests++;
    contLinha = 1;

    // Envia a string para o scanner do Flex analisar
    YY_BUFFER_STATE state = yy_scan_string(input);
    int token_obtido = yylex();
    yy_delete_buffer(state);

    // Valida se o token retornado é igual ao token esperado
    if (token_obtido == expected_token)
    {
        printf(" \033[0;32m[PASS]\033[0m %s (Entrada: \"%s\") -> Token: %d\n", test_name, input, token_obtido);
        passed_tests++;
    }
    else
    {
        printf(" \033[0;31m[FAIL]\033[0m %s (Entrada: \"%s\")\n", test_name, input);
        printf("        -> Esperado Token ID: %d\n", expected_token);
        printf("        -> Obtido Token ID:   %d\n", token_obtido);
    }
}

int main(void)
{
    printf("\n        SUÍTE DE TESTES UNITÁRIOS - SCANNER      \n\n");

    /* --- TESTES DE PALAVRAS-CHAVE --- */
    run_test("Tipo int", "int", KW_INT);
    run_test("Tipo double", "double", KW_DOUBLE);
    run_test("Estrutura if", "if", KW_IF);
    run_test("Estrutura else", "else", KW_ELSE);
    run_test("Literal true", "true", KW_TRUE);

    /* --- TESTES DE OPERADORES --- */
    run_test("Operador Soma", "+", '+');
    run_test("Operador Igualdade", "==", OP_IGUAL);
    run_test("Operador Incremento", "++", OP_INC);
    run_test("Operador Diferença", "!=", OP_DIFERENTE);
    run_test("Operador E Lógico", "&&", OP_AND);

    /* --- TESTES DE DELIMITADORES --- */
    run_test("Abre Parênteses", "(", '(');
    run_test("Ponto e Vírgula", ";", ';');

    /* --- TESTES DE IDENTIFICADORES E LITERAIS --- */
    run_test("Identificador simples", "contador", ID);
    run_test("Identificador com número", "var123", ID);
    run_test("Número Inteiro", "1234", LIT_INT);
    run_test("Número Double", "10.5", LIT_DOUBLE);
    run_test("Literal String", "\"ola mundo\"", LIT_STRING);

    printf("\nResultado: %d/%d testes passaram.\n", passed_tests, total_tests);

    return (passed_tests == total_tests) ? 0 : 1;
}