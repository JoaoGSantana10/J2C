/* Declarações do Bison */

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern int contErroLex;

ASTNode *ast_raiz = NULL;
%}

%code requires {
    #include "ast.h"
}

%union {
    int ival;
    char *sval;
    ASTNode *node;
}

%type <sval> tipo nome_qualificado
%type <node> programa corpo_classe
%type <node> valor_literal expressao chamada_funcao argumento lista_argumentos argumentos_opt
%type <ival> modificador modificadores_opt
%type <node> declaracao decl_variavel decl_funcao param lista_parametros parametros_opt
%type <node> bloco lista_comandos comando atribuicao inc_dec comando_return
%type <node> comando_if comando_while comando_do_while comando_for comando_switch
%type <node> init_for expressao_opt pos_for secao_cases lista_cases elemento_case elemento_default

/* Habilita mensagens de erro sintático detalhadas */
%define parse.error verbose

/* Palavras-chave de classe e modificadores */
%token KW_PUBLIC
%token KW_CLASS
%token KW_STATIC
%token KW_PRIVATE
%token KW_PROTECTED

/* Operador de Criação de Objetos */
%token KW_NEW

/* Palavras-chave de tipo */
%token KW_VOID
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
%token KW_CONTINUE
%token KW_DEFAULT
%token KW_RETURN

/* Literais booleanos */
%token <sval> KW_TRUE 
%token <sval> KW_FALSE

/* Identificador */
%token <sval> ID

/* Literais de valor */
%token <sval> LIT_INT 
%token <sval> LIT_DOUBLE 
%token <sval> LIT_LONG 
%token <sval> LIT_FLOAT 
%token <sval> LIT_CHAR 
%token <sval> LIT_STRING

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

%precedence LOWER_THAN_ELSE
%precedence KW_ELSE

%left OP_OR
%left OP_AND
%left OP_IGUAL OP_DIFERENTE
%left '<' '>' OP_MAIOR_IGUAL OP_MENOR_IGUAL
%left '+' '-'
%left '*' '/' '%'
%precedence '!' UMINUS

%%

/* REGRAS GRAMATICAIS */

programa
    : KW_PUBLIC KW_CLASS ID '{' corpo_classe '}' {
        ASTNode *classe = ast_create_class(MOD_PUBLIC, $3, $5, yylineno);
        $$ = ast_create_program(classe, yylineno);
        ast_raiz = $$;
    }
;

corpo_classe
    : corpo_classe declaracao   { $$ = ast_append($1, $2); }
    | %empty                    { $$ = NULL; }
;

declaracao
    : modificadores_opt decl_variavel   { ast_set_modifiers($2, $1); $$ = $2; }
    | modificadores_opt decl_funcao     { ast_set_modifiers($2, $1); $$ = $2; }
;

modificador
    : KW_PUBLIC                         { $$ = MOD_PUBLIC; }
    | KW_PRIVATE                        { $$ = MOD_PRIVATE; }
    | KW_PROTECTED                      { $$ = MOD_PROTECTED; }
    | KW_STATIC                         { $$ = MOD_STATIC; }
;

modificadores_opt
    : modificadores_opt modificador     { $$ = $1 | $2; }
    | %empty                            { $$ = MOD_NONE; }
;

tipo
    : KW_INT            { $$ = ast_safe_strdup("int"); }
    | KW_DOUBLE         { $$ = ast_safe_strdup("double"); }
    | KW_FLOAT          { $$ = ast_safe_strdup("float"); }
    | KW_BOOLEAN        { $$ = ast_safe_strdup("boolean"); }
    | KW_CHAR           { $$ = ast_safe_strdup("char"); }
    | KW_LONG           { $$ = ast_safe_strdup("long"); }
    | ID                { $$ = $1; }
;

valor_literal
    : LIT_INT           { $$ = ast_create_literal(NODE_LITERAL_INT, $1, yylineno); }
    | LIT_LONG          { $$ = ast_create_literal(NODE_LITERAL_LONG, $1, yylineno); }
    | LIT_DOUBLE        { $$ = ast_create_literal(NODE_LITERAL_DOUBLE, $1, yylineno); }
    | LIT_STRING        { $$ = ast_create_literal(NODE_LITERAL_STRING, $1, yylineno); }
    | LIT_FLOAT         { $$ = ast_create_literal(NODE_LITERAL_FLOAT, $1, yylineno); }
    | LIT_CHAR          { $$ = ast_create_literal(NODE_LITERAL_CHAR, $1, yylineno); }
    | KW_TRUE           { $$ = ast_create_literal(NODE_LITERAL_BOOL, $1, yylineno); }
    | KW_FALSE          { $$ = ast_create_literal(NODE_LITERAL_BOOL, $1, yylineno); }
;

param
    : tipo ID                                       { $$ = ast_create_param($1, $2, 0, yylineno); }
    | ID '[' ']' ID                                 { $$ = ast_create_param($1, $4, 1, yylineno); }
; 

lista_parametros
    : lista_parametros ',' param                    { $$ = ast_append($1, $3); }
    | param                                         { $$ = $1; }
;

parametros_opt
    : %empty                                        { $$ = NULL; }
    | lista_parametros                              { $$ = $1; }
;

decl_variavel
    : tipo ID ';'                                   { $$ = ast_create_var_decl(MOD_NONE, $1, $2, NULL, yylineno); }
    | tipo ID '=' expressao ';'                     { $$ = ast_create_var_decl(MOD_NONE, $1, $2, $4, yylineno); }
;

decl_funcao
    : tipo ID '(' parametros_opt ')' bloco          { $$ = ast_create_func_decl(MOD_NONE, $1, $2, $4, $6, yylineno); }
    | KW_VOID ID '(' parametros_opt ')' bloco       { $$ = ast_create_func_decl(MOD_NONE, ast_safe_strdup("void"), $2, $4, $6, yylineno); }
;

inc_dec
    : OP_INC ID                                     { $$ = ast_create_unary_op(AST_UN_INC, 0, ast_create_identifier($2, yylineno), yylineno); }
    | ID OP_INC                                     { $$ = ast_create_unary_op(AST_UN_INC, 1, ast_create_identifier($1, yylineno), yylineno); }
    | OP_DEC ID                                     { $$ = ast_create_unary_op(AST_UN_DEC, 0, ast_create_identifier($2, yylineno), yylineno); }
    | ID OP_DEC                                     { $$ = ast_create_unary_op(AST_UN_DEC, 1, ast_create_identifier($1, yylineno), yylineno); }
;

bloco
    : '{' lista_comandos '}'                        { $$ = ast_create_block($2, yylineno); }
;

lista_comandos
    : lista_comandos comando                        { $$ = ast_append($1, $2); }
    | %empty                                        { $$ = NULL; }
;

/* COMANDOS */

comando
    : decl_variavel                                 { $$ = $1; }
    | atribuicao ';'                                { $$ = $1; }
    | chamada_funcao ';'                            { $$ = ast_create_expr_stmt($1, yylineno); }
    | comando_return ';'                            { $$ = $1; }
    | comando_if                                    { $$ = $1; }
    | comando_while                                 { $$ = $1; }
    | comando_do_while                              { $$ = $1; }
    | comando_for                                   { $$ = $1; }
    | comando_switch                                { $$ = $1; }
    | KW_BREAK ';'                                  { $$ = ast_create_break(yylineno); }
    | KW_CONTINUE ';'                               { $$ = ast_create_continue(yylineno); }
    | bloco                                         { $$ = $1; }
    | inc_dec ';'                                   { $$ = ast_create_expr_stmt($1, yylineno); }
;

atribuicao
    : ID '=' expressao                              { $$ = ast_create_assign(AST_ASG_ASSIGN, $1, $3, yylineno); }
    | ID OP_PLUS_ASSIGN expressao                   { $$ = ast_create_assign(AST_ASG_PLUS_ASSIGN, $1, $3, yylineno); }
    | ID OP_MINUS_ASSIGN expressao                  { $$ = ast_create_assign(AST_ASG_MINUS_ASSIGN, $1, $3, yylineno); }
;

argumento
    : expressao                                 { $$ = $1; }
;

lista_argumentos
    : lista_argumentos ',' argumento            { $$ = ast_append($1, $3); }
    | argumento                                 { $$ = $1; }
;

argumentos_opt
    : %empty                                    { $$ = NULL; }
    | lista_argumentos                          { $$ = $1; }
;

chamada_funcao
    : nome_qualificado '(' argumentos_opt ')'   { $$ = ast_create_call($1, $3, yylineno); }
;

nome_qualificado
    : nome_qualificado '.' ID                   { $$ = ast_join_qualified($1, $3); }
    | ID                                        { $$ = $1; }
;

comando_return
    : KW_RETURN expressao                           { $$ = ast_create_return($2, yylineno); }
    | KW_RETURN                                     { $$ = ast_create_return(NULL, yylineno); }
;

/* --- ESTRUTURAS DE CONTROLE --- */

comando_if
    : KW_IF '(' expressao ')' comando %prec LOWER_THAN_ELSE { $$ = ast_create_if($3, $5, NULL, yylineno); }
    | KW_IF '(' expressao ')' comando KW_ELSE comando       { $$ = ast_create_if($3, $5, $7, yylineno); }
;

comando_while
    : KW_WHILE '(' expressao ')' comando                    { $$ = ast_create_while($3, $5, yylineno); }
;

comando_do_while
    : KW_DO comando KW_WHILE '(' expressao ')' ';'          { $$ = ast_create_do_while($2, $5, yylineno); }
;

comando_for
    : KW_FOR '(' init_for ';' expressao_opt ';' pos_for ')' comando { $$ = ast_create_for($3, $5, $7, $9, yylineno); }
;

init_for
    : tipo ID '=' expressao                                 { $$ = ast_create_var_decl(MOD_NONE, $1, $2, $4, yylineno); }
    | atribuicao                                            { $$ = $1; }
    | %empty                                                { $$ = NULL; }
;

expressao_opt
    : expressao                                             { $$ = $1; }
    | %empty                                                { $$ = NULL; }
;

pos_for
    : atribuicao                                            { $$ = $1; }
    | expressao                                             { $$ = $1; }
    | %empty                                                { $$ = NULL; }
;

comando_switch
    : KW_SWITCH '(' expressao ')' '{' secao_cases '}'       { $$ = ast_create_switch($3, $6, yylineno); }
;

secao_cases
    : lista_cases                                           { $$ = $1; }
    | lista_cases elemento_default                          { $$ = ast_append($1, $2); }
;

lista_cases
    : lista_cases elemento_case                             { $$ = ast_append($1, $2); }
    | %empty                                                { $$ = NULL; }
;

elemento_case
    : KW_CASE valor_literal ':' lista_comandos              { $$ = ast_create_case($2, $4, yylineno); }
;

elemento_default
    : KW_DEFAULT ':' lista_comandos                         { $$ = ast_create_default($3, yylineno); }
;

/* --- EXPRESSÕES E PRECEDÊNCIA --- */

expressao
    : expressao '+' expressao               { $$ = ast_create_bin_op(AST_OP_ADD, $1, $3, yylineno); }
    | expressao '-' expressao               { $$ = ast_create_bin_op(AST_OP_SUB, $1, $3, yylineno); }
    | expressao '*' expressao               { $$ = ast_create_bin_op(AST_OP_MUL, $1, $3, yylineno); }
    | expressao '/' expressao               { $$ = ast_create_bin_op(AST_OP_DIV, $1, $3, yylineno); }
    | expressao '%' expressao               { $$ = ast_create_bin_op(AST_OP_MOD, $1, $3, yylineno); }
    | expressao OP_IGUAL expressao          { $$ = ast_create_bin_op(AST_OP_EQ, $1, $3, yylineno); }
    | expressao OP_DIFERENTE expressao      { $$ = ast_create_bin_op(AST_OP_NE, $1, $3, yylineno); }
    | expressao '>' expressao               { $$ = ast_create_bin_op(AST_OP_GT, $1, $3, yylineno); }
    | expressao '<' expressao               { $$ = ast_create_bin_op(AST_OP_LT, $1, $3, yylineno); }
    | expressao OP_MAIOR_IGUAL expressao    { $$ = ast_create_bin_op(AST_OP_GE, $1, $3, yylineno); }
    | expressao OP_MENOR_IGUAL expressao    { $$ = ast_create_bin_op(AST_OP_LE, $1, $3, yylineno); }
    | expressao OP_AND expressao            { $$ = ast_create_bin_op(AST_OP_AND, $1, $3, yylineno); }
    | expressao OP_OR expressao             { $$ = ast_create_bin_op(AST_OP_OR, $1, $3, yylineno); }
    | '!' expressao                         { $$ = ast_create_unary_op(AST_UN_NOT, 0, $2, yylineno); }
    | '-' expressao %prec UMINUS            { $$ = ast_create_unary_op(AST_UN_NEG, 0, $2, yylineno); }
    | OP_INC ID                             { $$ = ast_create_unary_op(AST_UN_INC, 0, ast_create_identifier($2, yylineno), yylineno); }
    | ID OP_INC                             { $$ = ast_create_unary_op(AST_UN_INC, 1, ast_create_identifier($1, yylineno), yylineno); }
    | OP_DEC ID                             { $$ = ast_create_unary_op(AST_UN_DEC, 0, ast_create_identifier($2, yylineno), yylineno); }
    | ID OP_DEC                             { $$ = ast_create_unary_op(AST_UN_DEC, 1, ast_create_identifier($1, yylineno), yylineno); }
    | '(' expressao ')'                     { $$ = $2; }
    | ID                                    { $$ = ast_create_identifier($1, yylineno); }
    | valor_literal                         { $$ = $1; }
    | chamada_funcao                        { $$ = $1; }
    | KW_NEW ID '(' nome_qualificado ')'    { $$ = ast_create_new($2, $4, yylineno); }
;

%%

/* Código C auxiliar */

void yyerror(const char *s) {
    fprintf(stderr, "Erro de sintaxe: %s perto da linha %d\n", s, yylineno);
}

int main(void) {
    int result = yyparse();

    if (contErroLex > 0) {
        fprintf(stderr, "%s %d %s\n",
        (contErroLex == 1) ? "Foi encontrado" : "Foram encontrados",
        contErroLex,
        (contErroLex == 1) ? "erro léxico" : "erros léxicos");
        if (ast_raiz) {
            ast_free(ast_raiz);
        }
        return 1;
    }

    if (result == 0) {
        printf("Análise sintática concluída com sucesso!\n");
        if (ast_raiz) {
            ast_print(ast_raiz, 0);
            ast_free(ast_raiz);
        }
        return 0; /* Código 0: Sucesso */
    }

    if (ast_raiz) {
        ast_free(ast_raiz);
    }
    return 1;     /* Código 1: Erro de sintaxe */
}