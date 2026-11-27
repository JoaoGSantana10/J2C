#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"

#define CODEGEN_OK   0
#define CODEGEN_ERRO 1

/*
 * Gera codigo C a partir da AST em um fluxo ja aberto.
 * - Nao abre nem fecha `saida` (quem abriu e dono do fluxo).
 * - Nao libera a AST (a posse continua no main do parser).
 * - Retorna CODEGEN_OK ou CODEGEN_ERRO (mensagem vai para stderr).
 */
int codegen_gerar(const ASTNode *raiz, FILE *saida);

/*
 * Conveniencia para o main: grava em "<caminho>.tmp" e so renomeia para
 * `caminho` se a geracao terminar sem erro. Em falha, remove o .tmp,
 * entao nunca sobra arquivo parcial. Se o .tmp ja existir, falha sem
 * sobrescrever nada.
 */
int codegen_gerar_arquivo(const ASTNode *raiz, const char *caminho);

/*
 * Traduz um nome de tipo Java para o tipo C correspondente
 * (ex.: "long" -> "long long", "String" -> "char *").
 * Retorna NULL quando o tipo nao e suportado (inclusive tipos de classe).
 * O resultado e uma string estatica: nao deve ser liberada.
 */
const char *codegen_tipo(const char *tipo_java);

#endif
