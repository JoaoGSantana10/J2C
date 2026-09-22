%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"

int contLinha = 1;
%}

%option nounput noinput

DIGITO      [0-9]
LETRA       [a-zA-Z_$]
ALFANUM     [a-zA-Z_$0-9]

%%

"\n"        { contLinha++; }
[ \t\r]+    { /* Ignora espacos em branco */ }

"=="        { return OP_IGUAL; }
"!="        { return OP_DIFERENTE; }
">="        { return OP_MAIOR_IGUAL; }
"<="        { return OP_MENOR_IGUAL; }
"+="        { return OP_PLUS_ASSIGN; }
"-="        { return OP_MINUS_ASSIGN; }
"++"        { return OP_INC; }
"--"        { return OP_DEC; }
"&&"        { return OP_AND; }
"||"        { return OP_OR; }

"="         { return '='; }
"+"         { return '+'; }
"-"         { return '-'; }
"*"         { return '*'; }
"/"         { return '/'; }
"%"         { return '%'; }
">"         { return '>'; }
"<"         { return '<'; }
"!"         { return '!'; }

"("         { return '('; }
")"         { return ')'; }
"{"         { return '{'; }
"}"         { return '}'; }
"["         { return '['; }
"]"         { return ']'; }
"."         { return '.'; }
";"         { return ';'; }
","         { return ','; }
":"         { return ':'; }

"int"       { return KW_INT; }
"double"    { return KW_DOUBLE; }
"float"     { return KW_FLOAT; }
"boolean"   { return KW_BOOLEAN; }
"char"      { return KW_CHAR; }
"long"      { return KW_LONG; }
"if"        { return KW_IF; }
"else"      { return KW_ELSE; }
"while"     { return KW_WHILE; }
"for"       { return KW_FOR; }
"do"        { return KW_DO; }
"switch"    { return KW_SWITCH; }
"case"      { return KW_CASE; }
"break"     { return KW_BREAK; }
"default"   { return KW_DEFAULT; }
"return"    { return KW_RETURN; }
"true"      { return KW_TRUE; }
"false"     { return KW_FALSE; }

{DIGITO}+                   { return LIT_INT; }
{DIGITO}+"."{DIGITO}+       { return LIT_DOUBLE; }
{DIGITO}+"."{DIGITO}+[Ff]   { return LIT_FLOAT; }
{DIGITO}+[Ll]               { return LIT_LONG; }
"'"."'"                     { return LIT_CHAR; }
\".*\"                      { return LIT_STRING; }

{LETRA}{ALFANUM}*           { return ID; }

.                           { printf("Erro lexico na linha %d: caractere inesperado '%s'\n", contLinha, yytext); }

%%

int yywrap(void) {
    return 1;
}
