#include "symtab.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    Tabela *t = symtab_criar();
    Tipo ti = {TIPO_INT, NULL, 0};
    Tipo tc = {TIPO_CLASSE, "Pessoa", 0};
    Tipo tv = {TIPO_VOID, NULL, 0};

    assert(symtab_inserir(t, "Main", CAT_CLASSE, tc, 1));
    symtab_entrar_escopo(t); /* classe */
    assert(symtab_inserir(t, "x", CAT_CAMPO, ti, 2));
    assert(symtab_inserir(t, "main", CAT_METODO, tv, 3));
    symtab_entrar_escopo(t);                                     /* método */
    assert(symtab_inserir(t, "x", CAT_VARIAVEL, tc, 4));         /* shadowing */
    assert(symtab_inserir(t, "x", CAT_VARIAVEL, ti, 5) == NULL); /* redecl. */
    assert(symtab_buscar(t, "x")->categoria == CAT_VARIAVEL);
    assert(symtab_buscar_escopo_atual(t, "main") == NULL);
    assert(symtab_buscar(t, "main") != NULL);
    symtab_imprimir(t);
    symtab_sair_escopo(t);
    assert(symtab_buscar(t, "x")->categoria == CAT_CAMPO);
    assert(symtab_buscar(t, "y") == NULL);
    symtab_sair_escopo(t);
    assert(symtab_buscar(t, "x") == NULL);
    assert(symtab_buscar(t, "Main") != NULL);
    symtab_destruir(t);
    puts("OK");
    return 0;
}