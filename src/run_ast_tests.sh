#!/bin/bash

# Testes da impressao da AST: para cada tests/ast/X.java, compara a arvore
# impressa pelo parser com o conteudo de tests/ast/X.esperado.
# A primeira linha da saida (mensagem de sucesso) e ignorada.

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # Sem Cor

PASSED=0
FAILED=0

echo "       TESTES DA IMPRESSAO DA AST                "
echo -e "\n--- Arvore impressa x saida esperada ---"

for file in tests/ast/*.java; do
    [ -f "$file" ] || continue
    expected="${file%.java}.esperado"

    if [ ! -f "$expected" ]; then
        echo -e " ${RED}[FAIL]${NC} $file (arquivo $expected nao encontrado)"
        FAILED=$((FAILED + 1))
        continue
    fi

    output=$(./parser < "$file" 2>&1)
    status=$?

    if [ $status -ne 0 ]; then
        echo -e " ${RED}[FAIL]${NC} $file (parser retornou codigo $status)"
        echo "$output"
        FAILED=$((FAILED + 1))
        continue
    fi

    # --strip-trailing-cr: aceita .esperado com CRLF (checkout no Windows)
    if diff_output=$(echo "$output" | tail -n +2 | diff -u --strip-trailing-cr "$expected" -); then
        echo -e " ${GREEN}[PASS]${NC} $file"
        PASSED=$((PASSED + 1))
    else
        echo -e " ${RED}[FAIL]${NC} $file (arvore diferente do esperado)"
        echo "$diff_output"
        FAILED=$((FAILED + 1))
    fi
done

echo "Resultado Final: $PASSED passou, $FAILED falhou."

if [ $FAILED -ne 0 ]; then
    exit 1
fi
