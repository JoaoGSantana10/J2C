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
%token KW_RETURN

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

tipo
    : KW_INT 
    | KW_DOUBLE 
    | KW_FLOAT 
    | KW_BOOLEAN 
    | KW_CHAR 
    | KW_LONG
;

valor_literal
    : LIT_INT
    | LIT_LONG
    | LIT_DOUBLE
    | LIT_STRING
    | LIT_FLOAT
    | LIT_CHAR
    | KW_TRUE
    | KW_FALSE
;

param
    : tipo ID
; 

lista_parametros
    : lista_parametros ',' param
    | param
    | /* vazio */
;

decl_variavel
    : tipo ID ';'
    | tipo ID '=' valor_literal ';'
;

decl_funcao
    : tipo ID '('  lista_parametros ')' '{' lista_comandos '}'
;

lista_comandos
    : lista_comandos comando
    | /* vazio */
;

atribuicao
    : ID '=' valor_literal ';'
    | ID '=' ID ';'
;

argumento
    : valor_literal
    | ID
;

lista_argumentos
    : lista_argumentos ',' argumento
    | argumento
    | /* vazio */
;

chamada_funcao
    : ID '(' lista_argumentos ')' ';'
;

comando_return
    : KW_RETURN ID ';'
    | KW_RETURN valor_literal ';'
;

comando 
    : atribuicao
    | chamada_funcao
    | comando_return
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
