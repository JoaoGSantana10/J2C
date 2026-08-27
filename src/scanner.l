%{
#include <stdio.h>
#include <stdlib.h>
int contLinha = 1;
/* aqui entram includes extras, ex: "y.tab.h" quando o Bison estiver pronto */
%}

/* Seção de definições: nomes reutilizáveis para expressões regulares (opcional) */
DIGITO      [0-9]
LETRA       [a-zA-Z_$]
ALFANUM     [a-zA-Z_$0-9]

%%

{DIGITO}+ {printf ("Número encontrado %s\n", yytext);}
\n        {contLinha++;}
[ \t]+    {/*Ignora espaços em branco*/}

"+"    {printf("Operador soma encontrado %s\n",  yytext);}
"-"    {printf("Operador subtração encontrado %s\n",  yytext);}
"*"    {printf("Operador multiplicação encontrado %s\n",  yytext);}
"/"    {printf("Operador divisão encontrado %s\n",  yytext);}
"%"    {printf("Operador módulo encontrado %s\n",  yytext);}
"="    {printf("Operador atribuição encontrado %s\n",  yytext);}
"=="   {printf("Operador igualdade encontrado %s\n",  yytext);}
">"    {printf("Operador maior que encontrado %s\n",  yytext);}
"<"    {printf("Operador menor que encontrado %s\n",  yytext);}
">="   {printf("Operador maior igual encontrado %s\n",  yytext);}
"<="   {printf("Operador menor igual encontrado %s\n",  yytext);}
"!="   {printf("Operador deferença encontrado %s\n",  yytext);}
"&&"   {printf("Operador E lógico encontrado %s\n",  yytext);}
"||"   {printf("Operador OU lógico encontrado %s\n",  yytext);}
"!"    {printf("Operador negação encontrado %s\n",  yytext);}
"+="   {printf("Operador soma-atribuição encontrado %s\n",  yytext);}
"-="   {printf("Operador subtração-atribuição encontrado %s\n",  yytext);}
"++"   {printf("Operador incremento encontrado %s\n",  yytext);}
"--"   {printf("Operador decremento encontrado %s\n",  yytext);}

"("    {printf("Abre parênteses encontrado %s\n", yytext);}
")"    {printf("Fecha parênteses encontrado %s\n", yytext);}
"{"    {printf("Abre chaves encontrado %s\n", yytext);}
"}"    {printf("Fecha chaves encontrado %s\n", yytext);}
"["    {printf("Abre colchetes encontrado %s\n", yytext);}
"]"    {printf("Fecha colchetes encontrado %s\n", yytext);}
"."    {printf("Ponto encontrado %s\n", yytext);}
";"    {printf("Ponto e vírgula encontrado %s\n", yytext);}
","    {printf("Vírgula encontrado %s\n", yytext);}

"int"     {printf("Palavra-chave tipo int encontrada %s\n", yytext);}
"double"  {printf("Palavra-chave tipo double encontrada %s\n", yytext);}
"float"   {printf("Palavra-chave tipo float encontrada %s\n", yytext);}  
"boolean" {printf("Palavra-chave tipo boolean encontrada %s\n", yytext);} 
"char"    {printf("Palavra-chave tipo char encontrada %s\n", yytext);}
"long"    {printf("Palavra-chave tipo long encontrada %s\n", yytext);}
"if"      {printf("Palavra-chave tipo if encontrada %s\n", yytext);}
"else"    {printf("Palavra-chave tipo else encontrada %s\n", yytext);}
"while"   {printf("Palavra-chave tipo while encontrada %s\n", yytext);}
"for"     {printf("Palavra-chave tipo for encontrada %s\n", yytext);}
"do"      {printf("Palavra-chave tipo do encontrada %s\n", yytext);}
"switch"  {printf("Palavra-chave tipo switch encontrada %s\n", yytext);}
"case"    {printf("Palavra-chave tipo case encontrada %s\n", yytext);}
"break"   {printf("Palavra-chave tipo break encontrada %s\n", yytext);}
"default" {printf("Palavra-chave tipo default encontrada %s\n", yytext);}
"true"    {printf("Literal booleano true encontrado %s\n", yytext);}
"false"   {printf("Literal booleano false encontrado %s\n", yytext);}


{LETRA}{ALFANUM}* {printf("Identificador encontrado: %s\n", yytext);}

%%


int yywrap(void) {
    return 1;
}

int main(void) {
    yylex();
    printf("\nNumero de linhas %d\n", contLinha);
    return 0;
}