/* Declarações do Bison */

%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

/* Habilita mensagens de erro sintático detalhadas */
%define parse.error verbose

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

/* Literais booleanos */
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

/* PRECEDÊNCIA E ASSOCIAÇÃO DE OPERADORES */

%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

%right '=' OP_PLUS_ASSIGN OP_MINUS_ASSIGN
%left OP_OR
%left OP_AND
%left OP_IGUAL OP_DIFERENTE
%left '<' '>' OP_MAIOR_IGUAL OP_MENOR_IGUAL
%left '+' '-'
%left '*' '/' '%'
%right OP_INC OP_DEC '!' UMINUS

%%

/* REGRAS GRAMATICAIS */
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
    | tipo ID '=' expressao ';'
;

decl_funcao
    : tipo ID '(' lista_parametros ')' bloco
;

bloco
    : '{' lista_comandos '}'
;

lista_comandos
    : lista_comandos comando
    | /* vazio */
;

/* COMANDOS */

comando
    : decl_variavel
    | atribuicao ';'
    | chamada_funcao ';'
    | comando_return ';'
    | comando_if
    | comando_while
    | comando_do_while
    | comando_for
    | comando_switch
    | KW_BREAK ';'
    | bloco
;

atribuicao
    : ID '=' expressao
    | ID OP_PLUS_ASSIGN expressao
    | ID OP_MINUS_ASSIGN expressao
;

argumento
    : expressao
;

lista_argumentos
    : lista_argumentos ',' argumento
    | argumento
    | /* vazio */
;

chamada_funcao
    : ID '(' lista_argumentos ')'
;

comando_return
    : KW_RETURN expressao
    | KW_RETURN
;

/* --- ESTRUTURAS DE CONTROLE --- */

comando_if
    : KW_IF '(' expressao ')' comando %prec LOWER_THAN_ELSE
    | KW_IF '(' expressao ')' comando KW_ELSE comando
;

comando_while
    : KW_WHILE '(' expressao ')' comando
;

comando_do_while
    : KW_DO comando KW_WHILE '(' expressao ')' ';'
;

comando_for
    : KW_FOR '(' init_for ';' expressao_opt ';' pos_for ')' comando
;

init_for
    : tipo ID '=' expressao
    | atribuicao
    | /* vazio */
;

expressao_opt
    : expressao
    | /* vazio */
;

pos_for
    : atribuicao
    | expressao
    | /* vazio */
;

comando_switch
    : KW_SWITCH '(' expressao ')' '{' secao_cases '}'
;

secao_cases
    : lista_cases
    | lista_cases elemento_default
;

lista_cases
    : lista_cases elemento_case
    | /* vazio */
;

elemento_case
    : KW_CASE valor_literal ':' lista_comandos
;

elemento_default
    : KW_DEFAULT ':' lista_comandos
;

/* --- EXPRESSÕES E PRECEDÊNCIA --- */

expressao
    : expressao '+' expressao
    | expressao '-' expressao
    | expressao '*' expressao
    | expressao '/' expressao
    | expressao '%' expressao
    | expressao OP_IGUAL expressao
    | expressao OP_DIFERENTE expressao
    | expressao '>' expressao
    | expressao '<' expressao
    | expressao OP_MAIOR_IGUAL expressao
    | expressao OP_MENOR_IGUAL expressao
    | expressao OP_AND expressao
    | expressao OP_OR expressao
    | '!' expressao
    | '-' expressao %prec UMINUS
    | OP_INC ID
    | ID OP_INC
    | OP_DEC ID
    | ID OP_DEC
    | '(' expressao ')'
    | ID
    | valor_literal
    | chamada_funcao
;

%%

/* Código C auxiliar */

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s\n", s);
}

int main(void) {
    if (yyparse() == 0) {
        printf("Análise sintática concluída com sucesso!\n");
    }
    return 0;
}