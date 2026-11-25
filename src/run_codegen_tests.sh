#!/bin/bash
# Ponta a ponta: Java -> ./parser -> C -> gcc.
# Validos: o parser deve gerar o C e o gcc deve compilá-lo sem avisos.
# Invalidos: o parser deve falhar e nao deixar arquivo C (nem .tmp).

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

PASSED=0
FAILED=0
SAIDA=$(mktemp -d)
trap 'rm -rf "$SAIDA"' EXIT

echo "       TESTES DE GERAÇÃO DE CÓDIGO (PONTA A PONTA)      "

echo -e "\n--- Testes Positivos (C gerado deve compilar) ---"
for entrada in tests/codegen/validos/*.java; do
    c="$SAIDA/$(basename "${entrada%.java}").c"
    if ./parser "$c" < "$entrada" > /dev/null 2>&1 \
       && gcc -Wall -Wno-unused-variable -Werror -c "$c" -o "$c.o" 2> "$SAIDA/gcc.txt"; then
        echo -e " ${GREEN}[PASS]${NC} $entrada"
        PASSED=$((PASSED + 1))
    else
        echo -e " ${RED}[FAIL]${NC} $entrada"
        cat "$SAIDA/gcc.txt" 2>/dev/null
        FAILED=$((FAILED + 1))
    fi
done

echo -e "\n--- Testes Negativos (geração deve falhar) ---"
for entrada in tests/codegen/invalidos/*.java; do
    c="$SAIDA/$(basename "${entrada%.java}").c"
    ./parser "$c" < "$entrada" > /dev/null 2>&1
    rc=$?
    if [ $rc -ne 0 ] && [ ! -e "$c" ] && [ ! -e "$c.tmp" ]; then
        echo -e " ${GREEN}[PASS]${NC} $entrada"
        PASSED=$((PASSED + 1))
    else
        echo -e " ${RED}[FAIL]${NC} $entrada (rc=$rc, sobrou arquivo?)"
        FAILED=$((FAILED + 1))
    fi
done

echo -e "\nResultado: $PASSED passaram, $FAILED falharam"
[ $FAILED -eq 0 ]
