%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"
int contLinha = 1;
/* aqui entram includes extras, ex: "y.tab.h" quando o Bison estiver pronto */
%}

/* Seção de definições: nomes reutilizáveis para expressões regulares (opcional) */
DIGITO      [0-9]
LETRA       [a-zA-Z_$]
ALFANUM     [a-zA-Z_$0-9]

%%

{DIGITO}+ {return LIT_INT;}
\n        {contLinha++;}
[ \t]+    {/*Ignora espaços em branco*/}

"+"    {return '+';}
"-"    {return '-';}
"*"    {return '*';}
"/"    {return '/';}
"%"    {return '%';}
"=="   {return OP_IGUAL;}
">"    {return '>';}
"<"    {return '<';}
">="   {return OP_MAIOR_IGUAL;}
"<="   {return OP_MENOR_IGUAL;}
"!="   {return OP_DIFERENTE;}
"&&"   {return OP_AND;}
"||"   {return OP_OR;}
"!"    {return '!';}
"+="   {return OP_PLUS_ASSIGN;}
"-="   {return OP_MINUS_ASSIGN;}
"++"   {return OP_INC;}
"--"   {return OP_DEC;}

"("    {return '(';}
")"    {return ')';}
"{"    {return '{';}
"}"    {return '}';}
"["    {return '[';}
"]"    {return ']';}
"."    {return '.';}
","    {return ',';}

"int"     {return KW_INT;}
"float"   {return KW_FLOAT;}  
"boolean" {return KW_BOOLEAN;} 
"char"    {return KW_CHAR;}
"long"    {return KW_LONG;}
"if"      {return KW_IF;}
"else"    {return KW_ELSE;}
"while"   {return KW_WHILE;}
"for"     {return KW_FOR;}
"do"      {return KW_DO;}
"switch"  {return KW_SWITCH;}
"case"    {return KW_CASE;}
"break"   {return KW_BREAK;}
"default" {return KW_DEFAULT;}
"true"    {return KW_TRUE;}
"false"   {return KW_FALSE;}


{LETRA}{ALFANUM}* {return ID;}
{DIGITO}+[Ll]     {return LIT_LONG;}
{DIGITO}+"."{DIGITO}+ {return LIT_DOUBLE;}
{DIGITO}+"."{DIGITO}+[Ff] {return LIT_FLOAT;}
"'"."'" {return LIT_CHAR;}
\".*\" {return LIT_STRING;}
. {printf("Erro léxico: caractere inesperado: %s\n", yytext);}


%%


int yywrap(void) {
    return 1;
}

#ifndef TEST_MODE
int main(void) {
    yylex();
    printf("\nNumero de linhas %d\n", contLinha);
    return 0;
}
#endif