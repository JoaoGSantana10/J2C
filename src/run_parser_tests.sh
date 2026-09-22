#!/bin/bash

# Cores para o terminal
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # Sem Cor

PASSED=0
FAILED=0

echo "       TESTES SINTÁTICOS (PARSER)                "

# 1. Executa os testes válidos (Esperado: SUCESSO / Código de retorno 0)
echo -e "\n--- Testes Positivos (Devem Passar) ---"
for file in tests/validos/*.java; do
    if [ -f "$file" ]; then
        ./parser < "$file" > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo -e " ${GREEN}[PASS]${NC} $file"
            PASSED=$((PASSED + 1))
        else
            echo -e " ${RED}[FAIL]${NC} $file (Deveria ter passado, mas falhou)"
            FAILED=$((FAILED + 1))
        fi
    fi
done

# 2. Executa os testes inválidos (Esperado: ERRO / Código de retorno diferente de 0)
echo -e "\n--- Testes Negativos (Devem Falhar) ---"
for file in tests/invalidos/*.java; do
    if [ -f "$file" ]; then
        ./parser < "$file" > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo -e " ${GREEN}[PASS]${NC} $file (Erro capturado corretamente)"
            PASSED=$((PASSED + 1))
        else
            echo -e " ${RED}[FAIL]${NC} $file (Deveria ter falhado, mas passou)"
            FAILED=$((FAILED + 1))
        fi
    fi
done

echo "Resultado Final: $PASSED passou, $FAILED falhou."

if [ $FAILED -ne 0 ]; then
    exit 1
fi