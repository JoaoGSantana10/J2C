#include "semantico.h"
#include "symtab.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 * Mapa de escopos (níveis da tabela de símbolos):
 *   0  global   -> nome da classe
 *   1  classe   -> campos e métodos
 *   2  método   -> parâmetros E o corpo do método (mesmo nível!)
 *   3+ blocos   -> cada '{ ... }' aninhado abre um nível novo
 */

/* Tudo que o percurso precisa carregar de uma chamada para outra. */
typedef struct
{
    Tabela *tab;
    int erros;
} Sem;

static void visitar(Sem *s, const ASTNode *n);
static void visitar_lista(Sem *s, const ASTNode *n);

/* ================= utilitários ================= */

static void erro(Sem *s, int linha, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "Erro semântico (linha %d): ", linha);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    s->erros++;
}

/* Converte o nome de tipo que veio da AST ("int", "String", "Pessoa"...)
 * para o Tipo da tabela de símbolos. Qualquer nome desconhecido é tratado
 * como classe. O símbolo copia nome_classe ao ser inserido, então apontar
 * para a string da AST aqui é seguro. */
static Tipo tipo_de(const char *nome, int dimensoes)
{
    static const struct
    {
        const char *nome;
        TipoBase base;
    } conhecidos[] = {
        {"int", TIPO_INT},
        {"long", TIPO_LONG},
        {"float", TIPO_FLOAT},
        {"double", TIPO_DOUBLE},
        {"boolean", TIPO_BOOLEAN},
        {"char", TIPO_CHAR},
        {"String", TIPO_STRING},
        {"void", TIPO_VOID},
    };

    Tipo t = {TIPO_CLASSE, nome, dimensoes};
    for (size_t i = 0; i < sizeof conhecidos / sizeof conhecidos[0]; i++)
    {
        if (strcmp(nome, conhecidos[i].nome) == 0)
        {
            t.base = conhecidos[i].base;
            t.nome_classe = NULL;
            break;
        }
    }
    return t;
}

static const char *rotulo(Categoria c)
{
    switch (c)
    {
    case CAT_VARIAVEL:
        return "variável";
    case CAT_PARAMETRO:
        return "parâmetro";
    case CAT_CAMPO:
        return "campo";
    case CAT_METODO:
        return "método";
    case CAT_CLASSE:
        return "classe";
    }
    return "símbolo";
}

static void abrir_escopo(Sem *s) { symtab_entrar_escopo(s->tab); }
static void fechar_escopo(Sem *s) { symtab_sair_escopo(s->tab); }

/* Declara um nome no escopo ATUAL. Se já existir um igual NESTE escopo,
 * é redeclaração. Um nome igual em escopo externo é permitido (sombreia). */
static void declarar(Sem *s, const char *nome, Categoria cat, Tipo tipo,
                     int linha)
{
    const Simbolo *anterior = symtab_buscar_escopo_atual(s->tab, nome);
    if (anterior)
    {
        erro(s, linha,
             "redeclaração de %s '%s' no mesmo escopo "
             "(primeira declaração na linha %d)",
             rotulo(cat), nome, anterior->linha);
        return;
    }
    if (!symtab_inserir(s->tab, nome, cat, tipo, linha))
        erro(s, linha, "memória insuficiente ao declarar '%s'", nome);
}

/* Usa um nome: ele precisa existir no escopo atual ou em algum externo. */
static void usar(Sem *s, const char *nome, int linha)
{
    if (!symtab_buscar(s->tab, nome))
        erro(s, linha, "variável '%s' não declarada neste escopo", nome);
}

/* ================= declarações ================= */

/* Método: abre UM escopo que vale para parâmetros + corpo.
 * Por isso não chamamos visitar() no bloco do corpo (isso abriria um
 * segundo escopo e 'int f(int a) { int a; }' passaria sem erro). */
static void visitar_metodo(Sem *s, const ASTNode *m)
{
    abrir_escopo(s);

    for (const ASTNode *p = m->func_decl.params; p; p = p->next)
        declarar(s, p->param.name, CAT_PARAMETRO,
                 tipo_de(p->param.type_name, p->param.is_array ? 1 : 0),
                 p->lineno);

    if (m->func_decl.body)
        visitar_lista(s, m->func_decl.body->block.statements);

    fechar_escopo(s);
}

/* Classe em DUAS passadas. Em Java um método pode usar um campo (ou chamar
 * outro método) declarado mais abaixo no arquivo, então primeiro registramos
 * todos os membros e só depois olhamos os corpos. */
static void visitar_classe(Sem *s, const ASTNode *c)
{
    declarar(s, c->class_decl.name, CAT_CLASSE,
             tipo_de(c->class_decl.name, 0), c->lineno);

    abrir_escopo(s);

    /* Passada 1: registrar campos e métodos */
    for (const ASTNode *m = c->class_decl.members; m; m = m->next)
    {
        if (m->type == NODE_VAR_DECL)
            declarar(s, m->var_decl.name, CAT_CAMPO,
                     tipo_de(m->var_decl.type_name, 0), m->lineno);
        else if (m->type == NODE_FUNC_DECL)
            /* LIMITAÇÃO: sobrecarga (dois métodos com o mesmo nome e
             * parâmetros diferentes) vai dar falso "redeclaração". Fica
             * fora desta issue. */
            declarar(s, m->func_decl.name, CAT_METODO,
                     tipo_de(m->func_decl.return_type, 0), m->lineno);
    }

    /* Passada 2: inicializadores de campos e corpos de métodos */
    for (const ASTNode *m = c->class_decl.members; m; m = m->next)
    {
        if (m->type == NODE_VAR_DECL)
            visitar(s, m->var_decl.init);
        else if (m->type == NODE_FUNC_DECL)
            visitar_metodo(s, m);
    }

    fechar_escopo(s);
}

/* ================= percurso ================= */

static void visitar_lista(Sem *s, const ASTNode *n)
{
    for (; n; n = n->next)
        visitar(s, n);
}

static void visitar(Sem *s, const ASTNode *n)
{
    if (!n)
        return;

    switch (n->type)
    {
    /* ---- estrutura ---- */
    case NODE_PROGRAM:
        visitar_lista(s, n->program.body);
        break;

    case NODE_CLASS:
        visitar_classe(s, n);
        break;

    case NODE_FUNC_DECL:
    case NODE_PARAM:
        /* tratados dentro de visitar_classe / visitar_metodo */
        break;

    /* ---- variável local (também serve para o init do 'for') ----
     * Visitamos o inicializador ANTES de declarar: assim
     * 'int x = x + 1;' reclama que x não existe, em vez de ler o próprio x. */
    case NODE_VAR_DECL:
        visitar(s, n->var_decl.init);
        declarar(s, n->var_decl.name, CAT_VARIAVEL,
                 tipo_de(n->var_decl.type_name, 0), n->lineno);
        break;

    /* ---- bloco: escopo novo ---- */
    case NODE_BLOCK:
        abrir_escopo(s);
        visitar_lista(s, n->block.statements);
        fechar_escopo(s);
        break;

    /* ---- uso de nomes ---- */
    case NODE_ASSIGN:
        usar(s, n->assign.var_name, n->lineno);
        visitar(s, n->assign.expr);
        break;

    case NODE_IDENTIFIER:
        usar(s, n->identifier.name, n->lineno);
        break;

    /* ---- comandos simples: só descem nos filhos ---- */
    case NODE_EXPR_STMT:
        visitar(s, n->expr_stmt.expr);
        break;

    case NODE_RETURN:
        visitar(s, n->return_stmt.expr);
        break;

    case NODE_IF:
        visitar(s, n->if_stmt.condition);
        visitar(s, n->if_stmt.then_branch);
        visitar(s, n->if_stmt.else_branch);
        break;

    case NODE_WHILE:
        visitar(s, n->while_stmt.condition);
        visitar(s, n->while_stmt.body);
        break;

    case NODE_DO_WHILE:
        visitar(s, n->do_while_stmt.body);
        visitar(s, n->do_while_stmt.condition);
        break;

    case NODE_BREAK:
    case NODE_CONTINUE:
        break;

    /* ---- expressões ---- */
    case NODE_BIN_OP:
        visitar(s, n->bin_op.left);
        visitar(s, n->bin_op.right);
        break;

    case NODE_UNARY_OP:
        visitar(s, n->unary_op.operand);
        break;

    case NODE_LITERAL_INT:
    case NODE_LITERAL_DOUBLE:
    case NODE_LITERAL_FLOAT:
    case NODE_LITERAL_LONG:
    case NODE_LITERAL_CHAR:
    case NODE_LITERAL_STRING:
    case NODE_LITERAL_BOOL:
        break;

    /* ---- for: o 'int i' do init pertence ao for ----
     * O escopo do for envolve init, condição, incremento e corpo. Assim o
     * 'i' some quando o for termina, e dois for seguidos podem usar 'int i'.
     * Se o corpo for um bloco '{ }', ele abre o seu próprio escopo dentro
     * deste (como no Java). */
    case NODE_FOR:
        abrir_escopo(s);
        visitar(s, n->for_stmt.init);
        visitar(s, n->for_stmt.condition);
        visitar(s, n->for_stmt.increment);
        visitar(s, n->for_stmt.body);
        fechar_escopo(s);
        break;

    case NODE_SWITCH:
        visitar(s, n->switch_stmt.expr);
        abrir_escopo(s);
        visitar_lista(s, n->switch_stmt.cases);
        fechar_escopo(s);
        break;

    case NODE_CASE:
        visitar_lista(s, n->case_stmt.statements);
        break;

    case NODE_DEFAULT:
        visitar_lista(s, n->default_stmt.statements);
        break;

    case NODE_CALL:
        if (!strchr(n->call.func_name, '.') &&
            !symtab_buscar(s->tab, n->call.func_name))
            erro(s, n->lineno, "método '%s' não declarado",
                 n->call.func_name);
        visitar_lista(s, n->call.args);
        break;

    case NODE_NEW:
        if (n->new_expr.arg_name && !strchr(n->new_expr.arg_name, '.'))
            usar(s, n->new_expr.arg_name, n->lineno);
        break;
    }
}

/* ================= ponto de entrada ================= */

int semantico_analisar(const ASTNode *raiz)
{
    Sem s = {symtab_criar(), 0};
    if (!s.tab)
    {
        fprintf(stderr, "Erro semântico: memória insuficiente\n");
        return 1;
    }

    visitar(&s, raiz);

    symtab_destruir(s.tab);
    return s.erros;
}