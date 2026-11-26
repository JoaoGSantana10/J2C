#ifndef TRADUCAO_H
#define TRADUCAO_H

#include <stdio.h>
#include "ast.h"

void traduzir_comando(FILE *out, const ASTNode *node, int indent);
void traduzir_expressao(FILE *out, const ASTNode *node);

#endif /* TRADUCAO_H */
