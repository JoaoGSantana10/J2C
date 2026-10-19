#ifndef SYMTAB_H
#define SYMTAB_H

/* Categoria: o "papel" do identificador no programa */
typedef enum
{
    CAT_VARIAVEL,  /* variável local */
    CAT_PARAMETRO, /* parâmetro de método */
    CAT_CAMPO,     /* atributo de classe */
    CAT_METODO,
    CAT_CLASSE
} Categoria;

/* Tipo base. TIPO_CLASSE usa o campo nome_classe de Tipo. */
typedef enum
{
    TIPO_INT,
    TIPO_DOUBLE,
    TIPO_BOOLEAN,
    TIPO_CHAR,
    TIPO_STRING,
    TIPO_VOID,
    TIPO_CLASSE,
    TIPO_ERRO /* usado para evitar erros em cascata */
} TipoBase;

typedef struct
{
    TipoBase base;
    const char *nome_classe; /* só se base == TIPO_CLASSE (senão NULL) */
    int dimensoes;           /* 0 = escalar, 1 = int[], 2 = int[][] ... */
} Tipo;

/* Símbolo: uma entrada da tabela */
typedef struct Simbolo
{
    char *nome;
    Categoria categoria;
    Tipo tipo;            /* para métodos: tipo de retorno */
    int escopo;           /* nível em que foi declarado */
    int linha;            /* linha da declaração */
    struct Simbolo *prox; /* próximo da lista do balde (uso interno) */
} Simbolo;

/* Tipo opaco: o grupo não precisa conhecer a implementação */
typedef struct Tabela Tabela;

/* --- Criação / destruição ------------------------------------------ */
Tabela *symtab_criar(void);
void symtab_destruir(Tabela *t);

/* --- Escopo ---------------------------------------------------------- */
void symtab_entrar_escopo(Tabela *t);
void symtab_sair_escopo(Tabela *t); /* remove os símbolos do escopo */
int symtab_escopo_atual(const Tabela *t);

/* --- Inserção ---------------------------------------------------------
 * Insere no escopo atual. Retorna o símbolo criado, ou NULL se já existe
 * um símbolo com o mesmo nome NO MESMO escopo (redeclaração) ou se faltar
 * memória. Para saber a linha da declaração anterior, use
 * symtab_buscar_escopo_atual(). */
Simbolo *symtab_inserir(Tabela *t, const char *nome, Categoria cat,
                        Tipo tipo, int linha);

/* --- Busca ------------------------------------------------------------
 * symtab_buscar: do escopo atual para fora (o mais interno vence, como
 * no shadowing do Java). NULL se não declarado.
 * symtab_buscar_escopo_atual: só no escopo atual (para redeclaração). */
Simbolo *symtab_buscar(const Tabela *t, const char *nome);
Simbolo *symtab_buscar_escopo_atual(const Tabela *t, const char *nome);

/* --- Depuração ------------------------------------------------------- */
void symtab_imprimir(const Tabela *t);
const char *categoria_str(Categoria c);
const char *tipo_str(Tipo t); /* devolve buffer estático */

#endif /* SYMTAB_H */
