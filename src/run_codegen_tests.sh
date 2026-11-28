#!/bin/bash
# Ponta a ponta: Java -> ./parser -> C -> gcc.
# Validos: o parser deve gerar o C e o gcc deve compilá-lo sem avisos.
# Invalidos: o parser deve falhar e nao deixar arquivo C (nem .tmp).
# Execucao: o C gerado e linkado com um driver (X.c ao lado de X.java) que
#   chama as funcoes e confere se o resultado e o mesmo do Java. A main
#   gerada e renomeada (-Dmain=j2c_main) para a main do driver rodar.

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

echo -e "\n--- Testes de Execução (C gerado deve calcular o mesmo que o Java) ---"
for entrada in tests/codegen/execucao/*.java; do
    [ -f "$entrada" ] || continue
    driver="${entrada%.java}.c"
    base="$SAIDA/exec_$(basename "${entrada%.java}")"
    if ./parser "$base.c" < "$entrada" > /dev/null 2> "$SAIDA/erro.txt" \
       && gcc -Wall -Wno-unused-variable -Werror -Dmain=j2c_main \
              -c "$base.c" -o "$base.o" 2> "$SAIDA/gcc.txt" \
       && gcc -Wall -Werror "$driver" "$base.o" -o "$base" 2>> "$SAIDA/gcc.txt" \
       && "$base" > "$SAIDA/exec.txt"; then
        echo -e " ${GREEN}[PASS]${NC} $entrada"
        PASSED=$((PASSED + 1))
    else
        echo -e " ${RED}[FAIL]${NC} $entrada"
        cat "$SAIDA/erro.txt" "$SAIDA/gcc.txt" "$SAIDA/exec.txt" 2>/dev/null
        FAILED=$((FAILED + 1))
    fi
done

echo -e "\n--- Testes de Saída (programa gerado imprime o mesmo que o Java) ---"
for entrada in tests/codegen/saida/*.java; do
    [ -f "$entrada" ] || continue
    esperado="${entrada%.java}.saida"
    stdin="${entrada%.java}.entrada"
    [ -f "$stdin" ] || stdin=/dev/null
    base="$SAIDA/saida_$(basename "${entrada%.java}")"
    if ./parser "$base.c" < "$entrada" > /dev/null 2> "$SAIDA/erro.txt" \
       && gcc -Wall -Wno-unused-variable -Werror "$base.c" -o "$base" \
              2> "$SAIDA/gcc.txt" \
       && "$base" < "$stdin" > "$SAIDA/obtido.txt" \
       && diff "$esperado" "$SAIDA/obtido.txt" > "$SAIDA/diff.txt"; then
        echo -e " ${GREEN}[PASS]${NC} $entrada"
        PASSED=$((PASSED + 1))
    else
        echo -e " ${RED}[FAIL]${NC} $entrada"
        cat "$SAIDA/erro.txt" "$SAIDA/gcc.txt" "$SAIDA/diff.txt" 2>/dev/null
        FAILED=$((FAILED + 1))
    fi
done

echo -e "\nResultado: $PASSED passaram, $FAILED falharam"
[ $FAILED -eq 0 ]
