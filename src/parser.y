/* Declarações do Bison */

%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}


/* Palavras-chave de tipo */

%token KW_INT
%token KW_DOUBLE
%token KW_FLOAT
%token KW_BOOLEAN
%token KW_CHAR
%token KW_LONG

/* Palavras-chave de controle */

%token KW_IF 
%token KW_ELSE 
%token KW_WHILE 
%token KW_FOR 
%token KW_DO 
%token KW_SWITCH 
%token KW_CASE 
%token KW_BREAK 
%token KW_DEFAULT

/* Literais booleano */

%token KW_TRUE 
%token KW_FALSE

/* Identificador */

%token ID

/* Literais de valor */

%token LIT_INT 
%token LIT_DOUBLE 
%token LIT_LONG 
%token LIT_FLOAT 
%token LIT_CHAR 
%token LIT_STRING

/* Operadores Multi-caractere */

%token OP_IGUAL 
%token OP_DIFERENTE 
%token OP_MAIOR_IGUAL 
%token OP_MENOR_IGUAL 
%token OP_AND 
%token OP_OR 
%token OP_INC 
%token OP_DEC 
%token OP_PLUS_ASSIGN 
%token OP_MINUS_ASSIGN
%%


/* Regras de gramática */

programa
    : programa declaracao
    | declaracao
;
declaracao
    : decl_variavel
    | decl_funcao
; 


%%

/* Código C auxiliar */

void yyerror(const char *s){
    fprintf(stderr, "Erro de sintaxe: %s\n", s );
}

int main(void){
    yyparse();
    return 0;
}
