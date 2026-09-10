#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declarações internas do Flex */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yylex(void);
extern int contLinha;

int total_tests = 0;
int passed_tests = 0;

/* Função auxiliar que executa a análise em uma string e valida a saída do printf */
void run_test(const char *test_name, const char *input, const char *expected_output)
{
    total_tests++;
    contLinha = 1;

    // Redireciona temporariamente o stdout para um arquivo temporário em memória
    FILE *temp = tmpfile();
    if (!temp)
    {
        perror("Erro ao criar arquivo temporario para teste");
        exit(1);
    }

    FILE *old_stdout = stdout;
    stdout = temp;

    // Envia a string para o scanner do Flex analisar
    YY_BUFFER_STATE state = yy_scan_string(input);
    yylex();
    yy_delete_buffer(state);

    fflush(stdout);
    stdout = old_stdout;

    // Lê o conteúdo que foi impresso pelo lexer
    rewind(temp);
    char buffer[2048] = {0};
    fread(buffer, 1, sizeof(buffer) - 1, temp);
    fclose(temp);

    // Valida se a saída esperada está presente no texto gerado
    if (strstr(buffer, expected_output) != NULL)
    {
        printf(" \033[0;32m[PASS]\033[0m %s (Entrada: \"%s\")\n", test_name, input);
        passed_tests++;
    }
    else
    {
        printf(" \033[0;31m[FAIL]\033[0m %s (Entrada: \"%s\")\n", test_name, input);
        printf("        -> Esperado conter: \"%s\"\n", expected_output);
        printf("        -> Obtido:          \"%s\"\n", buffer);
    }
}

int main(void)
{
    printf("==================================================\n");
    printf("        SUÍTE DE TESTES UNITÁRIOS - SCANNER      \n");
    printf("==================================================\n\n");

    /* --- TESTES DE PALAVRAS-CHAVE --- */
    run_test("Tipo int", "int", "Palavra-chave tipo int encontrada int");
    run_test("Tipo double", "double", "Palavra-chave tipo double encontrada double");
    run_test("Estrutura if", "if", "Palavra-chave tipo if encontrada if");
    run_test("Estrutura else", "else", "Palavra-chave tipo else encontrada else");
    run_test("Literal true", "true", "Literal booleano true encontrado true");

    /* --- TESTES DE OPERADORES --- */
    run_test("Operador Soma", "+", "Operador soma encontrado +");
    run_test("Operador Igualdade", "==", "Operador igualdade encontrado ==");
    run_test("Operador Incremento", "++", "Operador incremento encontrado ++");
    run_test("Operador Diferença", "!=", "Operador deferença encontrado !=");
    run_test("Operador E Lógico", "&&", "Operador E lógico encontrado &&");

    /* --- TESTES DE DELIMITADORES --- */
    run_test("Abre Parênteses", "(", "Abre parênteses encontrado (");
    run_test("Ponto e Vírgula", ";", "Ponto e vírgula encontrado ;");

    /* --- TESTES DE IDENTIFICADORES E LITERAIS --- */
    run_test("Identificador simples", "contador", "Identificador encontrado: contador");
    run_test("Identificador com número", "var123", "Identificador encontrado: var123");
    run_test("Número Inteiro", "1234", "Número encontrado 1234");
    run_test("Número Double", "10.5", "Literal double encontrado: 10.5");
    run_test("Literal String", "\"ola mundo\"", "Literal string encontrado: \"ola mundo\"");

    /* --- TESTE DE ERRO LÉXICO --- */
    run_test("Caractere inválido", "@", "Erro léxico: caractere inesperado: @");

    printf("\n==================================================\n");
    printf("Resultado: %d/%d testes passaram.\n", passed_tests, total_tests);
    printf("==================================================\n");

    return (passed_tests == total_tests) ? 0 : 1;
}