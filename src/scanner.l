%{
#include <stdio.h>
#include <stdlib.h>
/* aqui entram includes extras, ex: "y.tab.h" quando o Bison estiver pronto */
%}

/* Seção de definições: nomes reutilizáveis para expressões regulares (opcional) */
DIGITO      [0-9]
LETRA       [a-zA-Z_]

%%

/* Seção de regras: padrão { ação }
   é aqui que você vai colocar espaços em branco, comentários,
   palavras-chave, identificadores, números, operadores, delimitadores */

%%

int yywrap(void) {
    return 1;
}

/* opcional: um main() de teste, só pra rodar o scanner sozinho
   antes de integrar com o parser */
int main(void) {
    yylex();
    return 0;
}