%{
#include <stdio.h>
#include <stdlib.h>
int contLinha = 1;
/* aqui entram includes extras, ex: "y.tab.h" quando o Bison estiver pronto */
%}

/* Seção de definições: nomes reutilizáveis para expressões regulares (opcional) */
DIGITO      [0-9]
LETRA       [a-zA-Z_]
OPERADORES  [\*, +, -, /, %, >=, <=, =, ==, !=, &&, ||,/]

%%
{DIGITO}+ {printf ("Número encontrado %s\n",yytext);}
\n        {contLinha++;}
[ \t]+    {/*Ignora espaços em branco*/}
{LETRA}+   {printf("Letra encontrada %s\n", yytext);}
%%


int yywrap(void) {
    return 1;
}

/* opcional: um main() de teste, só pra rodar o scanner sozinho
   antes de integrar com o parser */
int main(void) {
    yylex();
    printf("\nNumero de linhas %d\n", contLinha);
    return 0;
}