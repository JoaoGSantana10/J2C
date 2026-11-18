#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "ast.h"

/* Percorre a AST usando a tabela de símbolos (escopos, declarações e usos
 * de nomes). Imprime cada erro em stderr, no formato
 *   Erro semântico (linha N): mensagem
 * e devolve quantos erros foram encontrados (0 = programa ok). */
int semantico_analisar(const ASTNode *raiz);

#endif /* SEMANTICO_H */
