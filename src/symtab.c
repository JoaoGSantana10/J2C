#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_TABELA 211

struct Tabela
{
    Simbolo *baldes[TAM_TABELA];
    int escopo_atual;
};

/* ---------- utilitários internos ---------- */

static unsigned long hash_djb2(const char *s)
{
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*s++))
        h = ((h << 5) + h) + c; /* h = (h*32) + h */
    return h;
}

static char *copiar(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}

static void liberar_simbolo(Simbolo *s)
{
    free(s->nome);
    free((char *)s->tipo.nome_classe);
    free(s);
}

/* ---------- criação / destruição ---------- */

Tabela *symtab_criar(void)
{
    Tabela *t = calloc(1, sizeof(Tabela)); /* baldes = NULL, escopo = 0 */
    return t;
}

void symtab_destruir(Tabela *t)
{
    if (!t)
        return;
    for (int i = 0; i < TAM_TABELA; i++)
    {
        Simbolo *s = t->baldes[i];
        while (s)
        {
            Simbolo *prox = s->prox;
            liberar_simbolo(s);
            s = prox;
        }
    }
    free(t);
}

/* ---------- escopo ---------- */

void symtab_entrar_escopo(Tabela *t)
{
    t->escopo_atual++;
}

void symtab_sair_escopo(Tabela *t)
{
    if (t->escopo_atual == 0)
        return; /* o escopo global nunca sai */

    /* Como a inserção é sempre no início da lista, os símbolos do
     * escopo mais interno ficam na frente: basta remover da cabeça. */
    for (int i = 0; i < TAM_TABELA; i++)
    {
        while (t->baldes[i] && t->baldes[i]->escopo == t->escopo_atual)
        {
            Simbolo *morto = t->baldes[i];
            t->baldes[i] = morto->prox;
            liberar_simbolo(morto);
        }
    }
    t->escopo_atual--;
}

int symtab_escopo_atual(const Tabela *t)
{
    return t->escopo_atual;
}

/* ---------- inserção ---------- */

Simbolo *symtab_inserir(Tabela *t, const char *nome, Categoria cat,
                        Tipo tipo, int linha)
{
    if (symtab_buscar_escopo_atual(t, nome))
        return NULL; /* redeclaração */

    Simbolo *s = malloc(sizeof(Simbolo));
    if (!s)
        return NULL;

    s->nome = copiar(nome);
    s->categoria = cat;
    s->tipo = tipo;
    s->tipo.nome_classe = tipo.nome_classe ? copiar(tipo.nome_classe) : NULL;
    s->escopo = t->escopo_atual;
    s->linha = linha;

    if (!s->nome || (tipo.nome_classe && !s->tipo.nome_classe))
    {
        liberar_simbolo(s);
        return NULL;
    }

    unsigned long h = hash_djb2(nome) % TAM_TABELA;
    s->prox = t->baldes[h]; /* insere na cabeça */
    t->baldes[h] = s;
    return s;
}

/* ---------- busca ---------- */

Simbolo *symtab_buscar(const Tabela *t, const char *nome)
{
    unsigned long h = hash_djb2(nome) % TAM_TABELA;
    /* o primeiro encontrado é o mais interno (shadowing) */
    for (Simbolo *s = t->baldes[h]; s; s = s->prox)
        if (strcmp(s->nome, nome) == 0)
            return s;
    return NULL;
}

Simbolo *symtab_buscar_escopo_atual(const Tabela *t, const char *nome)
{
    unsigned long h = hash_djb2(nome) % TAM_TABELA;
    for (Simbolo *s = t->baldes[h]; s; s = s->prox)
    {
        if (s->escopo < t->escopo_atual)
            break; /* já passou dos símbolos do escopo atual */
        if (strcmp(s->nome, nome) == 0)
            return s;
    }
    return NULL;
}

/* ---------- depuração ---------- */

const char *categoria_str(Categoria c)
{
    switch (c)
    {
    case CAT_VARIAVEL:
        return "variavel";
    case CAT_PARAMETRO:
        return "parametro";
    case CAT_CAMPO:
        return "campo";
    case CAT_METODO:
        return "metodo";
    case CAT_CLASSE:
        return "classe";
    }
    return "?";
}

const char *tipo_str(Tipo t)
{
    static char buf[128];
    const char *base;
    switch (t.base)
    {
    case TIPO_INT:
        base = "int";
        break;
    case TIPO_DOUBLE:
        base = "double";
        break;
    case TIPO_BOOLEAN:
        base = "boolean";
        break;
    case TIPO_CHAR:
        base = "char";
        break;
    case TIPO_STRING:
        base = "String";
        break;
    case TIPO_VOID:
        base = "void";
        break;
    case TIPO_ERRO:
        base = "<erro>";
        break;
    case TIPO_CLASSE:
        base = t.nome_classe ? t.nome_classe : "?";
        break;
    default:
        base = "?";
        break;
    }
    int n = snprintf(buf, sizeof buf, "%s", base);
    for (int i = 0; i < t.dimensoes && n < (int)sizeof buf - 3; i++)
        n += snprintf(buf + n, sizeof buf - n, "[]");
    return buf;
}

void symtab_imprimir(const Tabela *t)
{
    printf("%-15s %-10s %-12s %-7s %s\n",
           "NOME", "CATEGORIA", "TIPO", "ESCOPO", "LINHA");
    for (int i = 0; i < TAM_TABELA; i++)
        for (Simbolo *s = t->baldes[i]; s; s = s->prox)
            printf("%-15s %-10s %-12s %-7d %d\n", s->nome,
                   categoria_str(s->categoria), tipo_str(s->tipo),
                   s->escopo, s->linha);
}
