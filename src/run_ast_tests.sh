#!/bin/bash
# Compara a AST impressa pelo parser com a saída esperada (.esperado).
# Os números de linha são removidos para o teste focar só na forma da árvore.

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

PASSED=0
FAILED=0

echo "       TESTES DA AST (FORMA DA ÁRVORE)          "

for entrada in tests/ast/*.java; do
    esperado="${entrada%.java}.esperado"

    if [ ! -f "$esperado" ]; then
        echo -e " ${RED}[FAIL]${NC} $entrada (arquivo $esperado não encontrado)"
        FAILED=$((FAILED + 1))
        continue
    fi
    
    # tr -d '\r' remove o final de linha do Windows (CRLF);
    # $(...) remove as linhas em branco do final dos dois textos.
    esperado_txt=$(tr -d '\r' < "$esperado")
    obtido=$(./parser < "$entrada" 2>&1 | sed 's/ (linha [0-9]*)//')

    if diff -u <(echo "$esperado_txt") <(echo "$obtido") > /tmp/ast_diff.txt; then
        echo -e " ${GREEN}[PASS]${NC} $entrada"
        PASSED=$((PASSED + 1))
    else
        echo -e " ${RED}[FAIL]${NC} $entrada (árvore diferente da esperada)"
        cat /tmp/ast_diff.txt
        FAILED=$((FAILED + 1))
    fi
done

echo "Resultado Final: $PASSED passou, $FAILED falhou."
[ $FAILED -eq 0 ]
