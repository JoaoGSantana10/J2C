#include "codegen.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Variável visível no ponto da geração (campo, parâmetro ou local). O
 * gerador não consulta a tabela de símbolos: ele só anota as declarações
 * que já percorreu na AST, para saber o tipo Java de cada nome. */
typedef struct {
    const char *nome;
    const char *tipo;    /* tipo Java, como está na AST ("int", "String"...) */
    int is_array;
} Visivel;

/* Estado de uma geração. Fica em uma struct (e não em globais) para o
 * gerador ser reentrante. */
typedef struct {
    FILE *saida;
    int erro;            /* 1 depois da primeira falha (AST ou escrita) */
    int tem_main;        /* já foi gerada uma main a partir do Java */
    int em_main;         /* dentro do corpo da main (return -> return 0) */
    int retorno_void;    /* dentro de uma função de retorno void */
    const ASTNode *membros;  /* membros da classe (para achar as funções) */
    Visivel *visiveis;   /* pilha: do escopo mais externo ao mais interno */
    size_t n_visiveis;
    size_t cap_visiveis;
} Ctx;

static void gen_no(Ctx *c, const ASTNode *no, int nivel);
static void gen_lista(Ctx *c, const ASTNode *lista, int nivel);
static void gen_expr(Ctx *c, const ASTNode *e, int como_comando);
static int eh_main_java(const ASTNode *f);

/* ---------------------------- erros -------------------------------- */

static void gen_erro(Ctx *c, int linha, const char *fmt, ...)
{
    va_list args;

    /* Só a primeira falha é relatada; as seguintes são consequência. */
    if (c->erro)
        return;
    c->erro = 1;

    if (linha > 0)
        fprintf(stderr, "Erro de geração de código na linha %d: ", linha);
    else
        fprintf(stderr, "Erro de geração de código: ");

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

/* ---------------------------- escrita ------------------------------ */

/* Única porta de saída: indenta, formata e detecta falha de escrita. */
static void emitir(Ctx *c, int nivel, const char *fmt, ...)
{
    va_list args;
    int i;

    if (c->erro)
        return;

    for (i = 0; i < nivel; i++) {
        if (fputs("    ", c->saida) == EOF) {
            gen_erro(c, 0, "impossível escrever no arquivo de saída");
            return;
        }
    }

    va_start(args, fmt);
    if (vfprintf(c->saida, fmt, args) < 0)
        gen_erro(c, 0, "impossível escrever no arquivo de saída");
    va_end(args);
}

/* ----------------------------- tipos ------------------------------- */

const char *codegen_tipo(const char *tipo_java)
{
    static const struct {
        const char *java;
        const char *c;
    } mapa[] = {
        {"int",     "int"},
        {"long",    "long long"},
        {"float",   "float"},
        {"double",  "double"},
        {"boolean", "bool"},
        {"char",    "char"},
        {"void",    "void"},
        {"String",  "char *"},
    };
    size_t i;

    if (tipo_java == NULL)
        return NULL;
    for (i = 0; i < sizeof mapa / sizeof mapa[0]; i++) {
        if (strcmp(tipo_java, mapa[i].java) == 0)
            return mapa[i].c;
    }
    return NULL;
}

/* Escreve "<tipo> <nome>" (ou "<tipo> *<nome>" para arrays), sem espaço
 * extra quando o tipo C já termina em '*' (ex.: "char *"). */
static void emitir_declarador(Ctx *c, const char *tipo_c, int is_array,
                              const char *nome)
{
    size_t n = strlen(tipo_c);
    int termina_em_ptr = (n > 0 && tipo_c[n - 1] == '*');

    emitir(c, 0, "%s%s%s%s", tipo_c, termina_em_ptr ? "" : " ",
           is_array ? "*" : "", nome);
}

/* Palavras que não podem ser nome em C (nem em conflito com <stdbool.h>). */
static int palavra_reservada_c(const char *nome)
{
    static const char *const reservadas[] = {
        "auto", "break", "case", "char", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "if", "inline", "int", "long", "register", "restrict", "return",
        "short", "signed", "sizeof", "static", "struct", "switch",
        "typedef", "union", "unsigned", "void", "volatile", "while",
        "bool", "true", "false",
    };
    size_t i;

    for (i = 0; i < sizeof reservadas / sizeof reservadas[0]; i++) {
        if (strcmp(nome, reservadas[i]) == 0)
            return 1;
    }
    return 0;
}

/* Valida o nome de uma declaração. Retorna 1 se serve. */
static int nome_valido(Ctx *c, const char *nome, int linha, const char *o_que)
{
    if (nome == NULL || nome[0] == '\0') {
        gen_erro(c, linha, "%s sem nome", o_que);
        return 0;
    }
    if (palavra_reservada_c(nome)) {
        gen_erro(c, linha, "%s '%s' usa palavra reservada de C", o_que, nome);
        return 0;
    }
    return 1;
}

/* Tipo C de uma declaração, ou NULL (com erro registrado). */
static const char *tipo_c_de(Ctx *c, const char *tipo_java, int linha)
{
    const char *t;

    if (tipo_java == NULL || tipo_java[0] == '\0') {
        gen_erro(c, linha, "declaração sem tipo");
        return NULL;
    }
    t = codegen_tipo(tipo_java);
    if (t == NULL)
        gen_erro(c, linha, "tipo '%s' não suportado", tipo_java);
    return t;
}

/* ------------------------ nomes visíveis --------------------------- */

static void declarar_visivel(Ctx *c, const char *nome, const char *tipo,
                             int is_array)
{
    if (c->erro || nome == NULL)
        return;
    if (c->n_visiveis == c->cap_visiveis) {
        size_t nova = c->cap_visiveis ? c->cap_visiveis * 2 : 16;
        Visivel *v = realloc(c->visiveis, nova * sizeof *v);

        if (v == NULL) {
            gen_erro(c, 0, "memória insuficiente");
            return;
        }
        c->visiveis = v;
        c->cap_visiveis = nova;
    }
    c->visiveis[c->n_visiveis].nome = nome;
    c->visiveis[c->n_visiveis].tipo = tipo;
    c->visiveis[c->n_visiveis].is_array = is_array;
    c->n_visiveis++;
}

/* Variável visível com esse nome (a mais interna), ou NULL. */
static const Visivel *busca_variavel(const Ctx *c, const char *nome)
{
    size_t i;

    if (nome == NULL)
        return NULL;
    for (i = c->n_visiveis; i > 0; i--) {
        if (strcmp(c->visiveis[i - 1].nome, nome) == 0)
            return &c->visiveis[i - 1];
    }
    return NULL;
}

/* Função da classe com esse nome, ou NULL. */
static const ASTNode *busca_funcao(const Ctx *c, const char *nome)
{
    const ASTNode *m;

    if (nome == NULL)
        return NULL;
    for (m = c->membros; m != NULL; m = m->next) {
        if (m->type == NODE_FUNC_DECL && m->func_decl.name != NULL &&
            strcmp(m->func_decl.name, nome) == 0)
            return m;
    }
    return NULL;
}

/* --------------------- tipo Java de uma expressão ------------------ */

/* Só o necessário para barrar traduções que mudariam o significado
 * (concatenação de String, % com ponto flutuante...). A checagem de tipos
 * completa é da análise semântica; aqui, na dúvida, o tipo é DESCONHECIDO
 * e nada é barrado. */
typedef enum {
    TE_DESCONHECIDO,
    TE_INT,
    TE_LONG,
    TE_FLOAT,
    TE_DOUBLE,
    TE_CHAR,
    TE_BOOLEAN,
    TE_STRING,
    TE_VOID,
    TE_OUTRO        /* arrays e classes */
} TipoExpr;

static TipoExpr tipo_de_nome(const char *tipo, int is_array)
{
    static const struct {
        const char *java;
        TipoExpr te;
    } mapa[] = {
        {"int", TE_INT},         {"long", TE_LONG},     {"float", TE_FLOAT},
        {"double", TE_DOUBLE},   {"char", TE_CHAR},     {"boolean", TE_BOOLEAN},
        {"String", TE_STRING},   {"void", TE_VOID},
    };
    size_t i;

    if (tipo == NULL)
        return TE_DESCONHECIDO;
    if (is_array)
        return TE_OUTRO;
    for (i = 0; i < sizeof mapa / sizeof mapa[0]; i++) {
        if (strcmp(tipo, mapa[i].java) == 0)
            return mapa[i].te;
    }
    return TE_OUTRO;
}

static int eh_numerico(TipoExpr t)
{
    return t == TE_INT || t == TE_LONG || t == TE_FLOAT || t == TE_DOUBLE ||
           t == TE_CHAR;
}

static int eh_ponto_flutuante(TipoExpr t)
{
    return t == TE_FLOAT || t == TE_DOUBLE;
}

static int eh_comparacao_ou_logico(BinaryOpType op)
{
    return op == AST_OP_EQ || op == AST_OP_NE || op == AST_OP_LT ||
           op == AST_OP_GT || op == AST_OP_LE || op == AST_OP_GE ||
           op == AST_OP_AND || op == AST_OP_OR;
}

/* Promoção numérica do Java: double > float > long > int (char vira int). */
static TipoExpr promove(TipoExpr a, TipoExpr b)
{
    if (a == TE_DOUBLE || b == TE_DOUBLE)
        return TE_DOUBLE;
    if (a == TE_FLOAT || b == TE_FLOAT)
        return TE_FLOAT;
    if (a == TE_LONG || b == TE_LONG)
        return TE_LONG;
    return TE_INT;
}

static TipoExpr tipo_expr(const Ctx *c, const ASTNode *e)
{
    const Visivel *v;
    const ASTNode *f;
    TipoExpr esq, dir;

    if (e == NULL)
        return TE_DESCONHECIDO;

    switch (e->type) {
    case NODE_LITERAL_INT:    return TE_INT;
    case NODE_LITERAL_LONG:   return TE_LONG;
    case NODE_LITERAL_FLOAT:  return TE_FLOAT;
    case NODE_LITERAL_DOUBLE: return TE_DOUBLE;
    case NODE_LITERAL_CHAR:   return TE_CHAR;
    case NODE_LITERAL_BOOL:   return TE_BOOLEAN;
    case NODE_LITERAL_STRING: return TE_STRING;

    case NODE_IDENTIFIER:
        v = busca_variavel(c, e->identifier.name);
        return v ? tipo_de_nome(v->tipo, v->is_array) : TE_DESCONHECIDO;

    case NODE_CALL:
        f = busca_funcao(c, e->call.func_name);
        return f ? tipo_de_nome(f->func_decl.return_type, 0) : TE_DESCONHECIDO;

    case NODE_UNARY_OP:
        if (e->unary_op.op == AST_UN_NOT)
            return TE_BOOLEAN;
        esq = tipo_expr(c, e->unary_op.operand);
        return (e->unary_op.op == AST_UN_NEG && esq == TE_CHAR) ? TE_INT : esq;

    case NODE_BIN_OP:
        if (eh_comparacao_ou_logico(e->bin_op.op))
            return TE_BOOLEAN;
        esq = tipo_expr(c, e->bin_op.left);
        dir = tipo_expr(c, e->bin_op.right);
        if (e->bin_op.op == AST_OP_ADD && (esq == TE_STRING || dir == TE_STRING))
            return TE_STRING;
        if (eh_numerico(esq) && eh_numerico(dir))
            return promove(esq, dir);
        return TE_DESCONHECIDO;

    default:
        return TE_DESCONHECIDO;
    }
}

/* ----------------------------- literais ---------------------------- */

static int so_digitos(const char *s, size_t n)
{
    size_t i;

    if (n == 0)
        return 0;
    for (i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '9')
            return 0;
    }
    return 1;
}

/* Java e C leem "010" como octal; "08" e "09" são inválidos nos dois. */
static int octal_invalido(const char *s, size_t n)
{
    return n > 1 && s[0] == '0' &&
           (memchr(s, '8', n) != NULL || memchr(s, '9', n) != NULL);
}

static void gen_literal_inteiro(Ctx *c, const ASTNode *e, const char *v,
                                size_t n, int eh_long)
{
    size_t digitos = n;

    /* 10L -> 10LL: em C o long pode ter só 32 bits; o long do Java tem 64,
     * como o long long (que é o tipo usado em codegen_tipo). */
    if (eh_long) {
        if (n < 2 || (v[n - 1] != 'L' && v[n - 1] != 'l')) {
            gen_erro(c, e->lineno, "literal long inválido '%s'", v);
            return;
        }
        digitos = n - 1;
    }
    if (!so_digitos(v, digitos) || octal_invalido(v, digitos)) {
        gen_erro(c, e->lineno, "literal inteiro inválido '%s'", v);
        return;
    }
    emitir(c, 0, "%.*s%s", (int)digitos, v, eh_long ? "LL" : "");
}

/* double: 3.14d -> 3.14 e 5d -> 5.0.   float: 1.5f -> 1.5f e 5f -> 5.0f.
 * Em C, "5f" e "5d" não são literais válidos. */
static void gen_literal_real(Ctx *c, const ASTNode *e, const char *v,
                             size_t n, int eh_float)
{
    const char *sufixos = eh_float ? "fF" : "dD";
    size_t m = n, i;
    int pontos = 0;

    if (m > 0 && strchr(sufixos, v[m - 1]) != NULL)
        m--;
    for (i = 0; i < m; i++) {
        if (v[i] == '.')
            pontos++;
        else if (v[i] < '0' || v[i] > '9')
            break;
    }
    if (m == 0 || i < m || pontos > 1 || v[0] == '.' || v[m - 1] == '.') {
        gen_erro(c, e->lineno, "literal %s inválido '%s'",
                 eh_float ? "float" : "double", v);
        return;
    }
    emitir(c, 0, "%.*s%s%s", (int)m, v, pontos ? "" : ".0",
           eh_float ? "f" : "");
}

/* O scanner aceita um caractere entre aspas simples, inclusive '\' e '''
 * (que em C precisam de escape). */
static void gen_literal_char(Ctx *c, const ASTNode *e, const char *v, size_t n)
{
    if (n != 3 || v[0] != '\'' || v[2] != '\'') {
        gen_erro(c, e->lineno, "literal char inválido %s", v);
        return;
    }
    if (v[1] == '\\')
        emitir(c, 0, "'\\\\'");
    else if (v[1] == '\'')
        emitir(c, 0, "'\\''");
    else
        emitir(c, 0, "%s", v);
}

static int valor_hex(char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

/* Escreve o código Unicode em UTF-8, byte a byte, como escapes octais. */
static void emitir_utf8(Ctx *c, unsigned long cp)
{
    unsigned char b[3];
    int n, i;

    if (cp < 0x80) {
        b[0] = (unsigned char)cp;
        n = 1;
    } else if (cp < 0x800) {
        b[0] = (unsigned char)(0xC0 | (cp >> 6));
        b[1] = (unsigned char)(0x80 | (cp & 0x3F));
        n = 2;
    } else {
        b[0] = (unsigned char)(0xE0 | (cp >> 12));
        b[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
        b[2] = (unsigned char)(0x80 | (cp & 0x3F));
        n = 3;
    }
    for (i = 0; i < n; i++)
        emitir(c, 0, "\\%03o", b[i]);
}

/* Mantém as aspas e os escapes comuns (\n, \t, \", \\ ...), que são iguais
 * em Java e C, e adapta o que é diferente:
 *  - \uXXXX (Java) vira os bytes UTF-8 em octal (C não aceita A);
 *  - octal do Java (\0 a \377) é reescrito com 3 dígitos, para o C não
 *    juntar o dígito seguinte ("\1" + "2" não pode virar "\12");
 *  - "??" vira "?\?" para não formar trígrafos do C;
 *  - uma barra sozinha no fim (o scanner fecha a string em \") vira \\. */
static void gen_literal_string(Ctx *c, const ASTNode *e, const char *v,
                               size_t n)
{
    size_t i, fim;

    if (n < 2 || v[0] != '"' || v[n - 1] != '"') {
        gen_erro(c, e->lineno, "literal String inválido %s", v);
        return;
    }
    fim = n - 1;
    emitir(c, 0, "\"");
    for (i = 1; i < fim && !c->erro; i++) {
        char ch = v[i];
        char prox = (i + 1 < fim) ? v[i + 1] : '\0';

        if (ch == '?' && prox == '?') {
            emitir(c, 0, "?\\");
        } else if (ch != '\\') {
            emitir(c, 0, "%c", ch);
        } else if (prox == '\0') {
            emitir(c, 0, "\\\\");
        } else if (strchr("btnfr\"'\\", prox) != NULL) {
            emitir(c, 0, "\\%c", prox);
            i++;
        } else if (prox >= '0' && prox <= '7') {
            /* Java: até 3 dígitos se o primeiro for 0-3; senão até 2. */
            int max = (prox <= '3') ? 3 : 2, k = 0, valor = 0;

            while (k < max && i + 1 < fim && v[i + 1] >= '0' && v[i + 1] <= '7') {
                valor = valor * 8 + (v[i + 1] - '0');
                i++;
                k++;
            }
            emitir(c, 0, "\\%03o", valor);
        } else if (prox == 'u') {
            unsigned long cp = 0;
            int k;

            i++;
            while (i + 1 < fim && v[i + 1] == 'u')   /* Java aceita \uuuu0041 */
                i++;
            for (k = 0; k < 4; k++) {
                int h = (i + 1 < fim) ? valor_hex(v[i + 1]) : -1;

                if (h < 0) {
                    gen_erro(c, e->lineno, "escape \\u incompleto em %s", v);
                    return;
                }
                cp = cp * 16 + (unsigned long)h;
                i++;
            }
            if (cp >= 0xD800 && cp <= 0xDFFF) {
                gen_erro(c, e->lineno, "escape \\u de par substituto (surrogate) "
                         "não suportado em %s", v);
                return;
            }
            emitir_utf8(c, cp);
        } else {
            gen_erro(c, e->lineno, "sequência de escape '\\%c' inválida em %s",
                     prox, v);
            return;
        }
    }
    emitir(c, 0, "\"");
}

static void gen_literal(Ctx *c, const ASTNode *e)
{
    const char *v = e->literal.value;
    size_t n;

    if (v == NULL || v[0] == '\0') {
        gen_erro(c, e->lineno, "literal sem valor");
        return;
    }
    n = strlen(v);

    switch (e->type) {
    case NODE_LITERAL_INT:    gen_literal_inteiro(c, e, v, n, 0); break;
    case NODE_LITERAL_LONG:   gen_literal_inteiro(c, e, v, n, 1); break;
    case NODE_LITERAL_DOUBLE: gen_literal_real(c, e, v, n, 0);    break;
    case NODE_LITERAL_FLOAT:  gen_literal_real(c, e, v, n, 1);    break;
    case NODE_LITERAL_CHAR:   gen_literal_char(c, e, v, n);       break;
    case NODE_LITERAL_STRING: gen_literal_string(c, e, v, n);     break;
    case NODE_LITERAL_BOOL:
        /* true/false do C vêm de <stdbool.h>, já incluído no arquivo. */
        if (strcmp(v, "true") == 0 || strcmp(v, "false") == 0)
            emitir(c, 0, "%s", v);
        else
            gen_erro(c, e->lineno, "literal boolean inválido '%s'", v);
        break;
    default:
        gen_erro(c, e->lineno, "nó não é um literal");
        break;
    }
}

/* ---------------------------- expressões --------------------------- */

static const char *op_binario_c(BinaryOpType op)
{
    switch (op) {
    case AST_OP_ADD: return "+";
    case AST_OP_SUB: return "-";
    case AST_OP_MUL: return "*";
    case AST_OP_DIV: return "/";
    case AST_OP_MOD: return "%";
    case AST_OP_EQ:  return "==";
    case AST_OP_NE:  return "!=";
    case AST_OP_LT:  return "<";
    case AST_OP_GT:  return ">";
    case AST_OP_LE:  return "<=";
    case AST_OP_GE:  return ">=";
    case AST_OP_AND: return "&&";
    case AST_OP_OR:  return "||";
    }
    return NULL;    /* valor fora do enum: AST corrompida */
}

static const char *op_unario_c(UnaryOpType op)
{
    switch (op) {
    case AST_UN_INC: return "++";
    case AST_UN_DEC: return "--";
    case AST_UN_NEG: return "-";
    case AST_UN_NOT: return "!";
    }
    return NULL;
}

static void gen_identificador(Ctx *c, const ASTNode *e)
{
    const char *nome = e->identifier.name;

    if (nome == NULL || nome[0] == '\0') {
        gen_erro(c, e->lineno, "identificador vazio");
        return;
    }
    if (busca_variavel(c, nome) == NULL && busca_funcao(c, nome) != NULL) {
        gen_erro(c, e->lineno, "'%s' é uma função e não pode ser usada como "
                 "valor", nome);
        return;
    }
    emitir(c, 0, "%s", nome);
}

/* Toda subexpressão binária vai entre parênteses: a + b * c vira
 * a + (b * c) e a - b - c vira (a - b) - c. Assim a precedência e a
 * associatividade que o parser montou na AST chegam intactas ao C, sem
 * depender da tabela de precedência do C. */
static void gen_operando(Ctx *c, const ASTNode *e, int parenteses_em_unario)
{
    int composto = e != NULL &&
        (e->type == NODE_BIN_OP ||
         (parenteses_em_unario && e->type == NODE_UNARY_OP));

    if (composto)
        emitir(c, 0, "(");
    gen_expr(c, e, 0);
    if (composto)
        emitir(c, 0, ")");
}

static void gen_bin_op(Ctx *c, const ASTNode *e)
{
    const char *op = op_binario_c(e->bin_op.op);
    TipoExpr esq, dir;

    if (op == NULL) {
        gen_erro(c, e->lineno, "operador binário desconhecido (%d)",
                 (int)e->bin_op.op);
        return;
    }
    if (e->bin_op.left == NULL || e->bin_op.right == NULL) {
        gen_erro(c, e->lineno, "operador '%s' sem os dois operandos", op);
        return;
    }

    esq = tipo_expr(c, e->bin_op.left);
    dir = tipo_expr(c, e->bin_op.right);
    if (esq == TE_STRING || dir == TE_STRING) {
        /* Em C, "a" + x é aritmética de ponteiro: compila, mas muda o
         * significado. == e != comparam referências nos dois. */
        if (e->bin_op.op == AST_OP_ADD) {
            gen_erro(c, e->lineno, "concatenação de String com '+' ainda não "
                     "é suportada");
            return;
        }
        if (e->bin_op.op != AST_OP_EQ && e->bin_op.op != AST_OP_NE) {
            gen_erro(c, e->lineno, "operador '%s' não pode ser usado com "
                     "String", op);
            return;
        }
    }
    if (e->bin_op.op == AST_OP_MOD &&
        (eh_ponto_flutuante(esq) || eh_ponto_flutuante(dir))) {
        gen_erro(c, e->lineno, "operador '%%' com float/double não existe em C");
        return;
    }

    gen_operando(c, e->bin_op.left, 0);
    emitir(c, 0, " %s ", op);
    gen_operando(c, e->bin_op.right, 0);
}

static void gen_unary_op(Ctx *c, const ASTNode *e)
{
    const char *op = op_unario_c(e->unary_op.op);
    const ASTNode *o = e->unary_op.operand;

    if (op == NULL) {
        gen_erro(c, e->lineno, "operador unário desconhecido (%d)",
                 (int)e->unary_op.op);
        return;
    }
    if (o == NULL) {
        gen_erro(c, e->lineno, "operador unário '%s' sem operando", op);
        return;
    }
    if (tipo_expr(c, o) == TE_STRING) {
        /* "++s" em C andaria o ponteiro da String. */
        gen_erro(c, e->lineno, "operador '%s' não pode ser usado com String", op);
        return;
    }

    if (e->unary_op.op == AST_UN_INC || e->unary_op.op == AST_UN_DEC) {
        if (o->type != NODE_IDENTIFIER) {
            gen_erro(c, e->lineno, "'%s' só pode ser aplicado a uma variável", op);
            return;
        }
        if (e->unary_op.is_postfix) {
            gen_expr(c, o, 0);
            emitir(c, 0, "%s", op);
        } else {
            emitir(c, 0, "%s", op);
            gen_expr(c, o, 0);
        }
        return;
    }

    /* Parênteses também em operando unário: -(-a) não pode virar --a. */
    emitir(c, 0, "%s", op);
    gen_operando(c, o, 1);
}

static void gen_chamada(Ctx *c, const ASTNode *e, int como_comando)
{
    const char *nome = e->call.func_name;
    const ASTNode *f, *a, *p;
    int n_args = 0, n_params = 0;

    if (nome == NULL || nome[0] == '\0') {
        gen_erro(c, e->lineno, "chamada sem nome");
        return;
    }
    if (strchr(nome, '.') != NULL) {
        /* System.out.println, sc.nextInt()... são da issue de entrada e
         * saída. A chamada nunca é descartada em silêncio. */
        gen_erro(c, e->lineno, "chamada qualificada '%s' ainda não é "
                 "suportada (será tratada pela issue de entrada e saída)", nome);
        return;
    }

    f = busca_funcao(c, nome);
    if (f == NULL) {
        gen_erro(c, e->lineno, "função '%s' não declarada na classe", nome);
        return;
    }
    if (eh_main_java(f)) {
        gen_erro(c, e->lineno, "a função main não pode ser chamada");
        return;
    }
    if (busca_variavel(c, nome) != NULL) {
        /* Java separa nomes de variáveis e de métodos; C não. */
        gen_erro(c, e->lineno, "a variável '%s' esconde a função de mesmo "
                 "nome em C", nome);
        return;
    }

    for (a = e->call.args; a != NULL; a = a->next)
        n_args++;
    for (p = f->func_decl.params; p != NULL; p = p->next)
        n_params++;
    if (n_args != n_params) {
        gen_erro(c, e->lineno, "função '%s' espera %d argumento(s), mas "
                 "recebeu %d", nome, n_params, n_args);
        return;
    }
    if (!como_comando &&
        tipo_de_nome(f->func_decl.return_type, 0) == TE_VOID) {
        gen_erro(c, e->lineno, "função void '%s' não pode ser usada como "
                 "valor", nome);
        return;
    }

    emitir(c, 0, "%s(", nome);
    for (a = e->call.args; a != NULL && !c->erro; a = a->next) {
        if (a != e->call.args)
            emitir(c, 0, ", ");
        gen_expr(c, a, 0);
    }
    emitir(c, 0, ")");
}

/* Estratégia para `new`: o J2C ainda não traduz classes para structs, então
 * todo `new` é rejeitado com uma mensagem clara, em vez de gerar C sem
 * sentido. `new Scanner(System.in)` será convertido pela issue de entrada
 * e saída. */
static void gen_new(Ctx *c, const ASTNode *e)
{
    const char *classe = e->new_expr.class_name;

    if (classe == NULL || classe[0] == '\0') {
        gen_erro(c, e->lineno, "new sem classe");
        return;
    }
    gen_erro(c, e->lineno, "'new %s(...)' não é suportado: classes ainda não "
             "são traduzidas para C", classe);
}

/* `como_comando`: a expressão é um comando inteiro (ex.: "f(x);"), então
 * pode ser uma chamada a função void. */
static void gen_expr(Ctx *c, const ASTNode *e, int como_comando)
{
    if (c->erro)
        return;
    if (e == NULL) {
        gen_erro(c, 0, "expressão ausente");
        return;
    }

    switch (e->type) {
    case NODE_LITERAL_INT:
    case NODE_LITERAL_LONG:
    case NODE_LITERAL_DOUBLE:
    case NODE_LITERAL_FLOAT:
    case NODE_LITERAL_CHAR:
    case NODE_LITERAL_STRING:
    case NODE_LITERAL_BOOL:
        gen_literal(c, e);
        break;
    case NODE_IDENTIFIER:
        gen_identificador(c, e);
        break;
    case NODE_BIN_OP:
        gen_bin_op(c, e);
        break;
    case NODE_UNARY_OP:
        gen_unary_op(c, e);
        break;
    case NODE_CALL:
        gen_chamada(c, e, como_comando);
        break;
    case NODE_NEW:
        gen_new(c, e);
        break;
    default:
        gen_erro(c, e->lineno, "expressão mal formada (nó não é uma expressão)");
        break;
    }
}

/* --------------- ordem de avaliação: Java x C ---------------------- */

/* Quantas vezes a variável `nome` aparece na expressão. */
static int conta_usos(const ASTNode *e, const char *nome)
{
    const ASTNode *a;
    int n = 0;

    if (e == NULL)
        return 0;
    switch (e->type) {
    case NODE_IDENTIFIER:
        return e->identifier.name != NULL && strcmp(e->identifier.name, nome) == 0;
    case NODE_BIN_OP:
        return conta_usos(e->bin_op.left, nome) + conta_usos(e->bin_op.right, nome);
    case NODE_UNARY_OP:
        return conta_usos(e->unary_op.operand, nome);
    case NODE_CALL:
        for (a = e->call.args; a != NULL; a = a->next)
            n += conta_usos(a, nome);
        return n;
    default:
        return 0;
    }
}

/* Procura um ++/-- cuja variável aparece de novo na expressão completa
 * (`raiz`) ou é o destino da atribuição (`alvo`). */
static const ASTNode *inc_dec_conflitante(const ASTNode *raiz,
                                          const ASTNode *e, const char *alvo)
{
    const ASTNode *a, *r;

    if (e == NULL)
        return NULL;
    switch (e->type) {
    case NODE_UNARY_OP:
        if ((e->unary_op.op == AST_UN_INC || e->unary_op.op == AST_UN_DEC) &&
            e->unary_op.operand != NULL &&
            e->unary_op.operand->type == NODE_IDENTIFIER &&
            e->unary_op.operand->identifier.name != NULL) {
            const char *nome = e->unary_op.operand->identifier.name;

            if (conta_usos(raiz, nome) > 1 ||
                (alvo != NULL && strcmp(alvo, nome) == 0))
                return e;
        }
        return inc_dec_conflitante(raiz, e->unary_op.operand, alvo);
    case NODE_BIN_OP:
        r = inc_dec_conflitante(raiz, e->bin_op.left, alvo);
        return r ? r : inc_dec_conflitante(raiz, e->bin_op.right, alvo);
    case NODE_CALL:
        for (a = e->call.args; a != NULL; a = a->next) {
            r = inc_dec_conflitante(raiz, a, alvo);
            if (r)
                return r;
        }
        return NULL;
    default:
        return NULL;
    }
}

/* Gera uma expressão completa (inicializador, lado direito, return,
 * comando). Java avalia tudo da esquerda para a direita; em C, modificar
 * uma variável com ++/-- e usá-la de novo na mesma expressão (i++ + i,
 * f(i, i++), i = i++) é comportamento indefinido. Esses casos são
 * rejeitados para o significado não mudar. */
static void gen_expr_completa(Ctx *c, const ASTNode *e, const char *alvo,
                              int como_comando)
{
    const ASTNode *x;

    if (c->erro)
        return;
    x = inc_dec_conflitante(e, e, alvo);
    if (x != NULL) {
        gen_erro(c, x->lineno, "'%s' altera '%s', que é usada de novo na mesma "
                 "expressão: em C isso tem comportamento indefinido",
                 op_unario_c(x->unary_op.op),
                 x->unary_op.operand->identifier.name);
        return;
    }
    gen_expr(c, e, como_comando);
}

/* "x = expr", "x += expr" ou "x -= expr", sem ';' (o for também usa). */
static void gen_atribuicao(Ctx *c, const ASTNode *no)
{
    const char *nome = no->assign.var_name;
    const char *op;
    const Visivel *v;

    switch (no->assign.op) {
    case AST_ASG_ASSIGN:       op = "=";  break;
    case AST_ASG_PLUS_ASSIGN:  op = "+="; break;
    case AST_ASG_MINUS_ASSIGN: op = "-="; break;
    default:
        gen_erro(c, no->lineno, "operador de atribuição desconhecido (%d)",
                 (int)no->assign.op);
        return;
    }
    if (nome == NULL || nome[0] == '\0') {
        gen_erro(c, no->lineno, "atribuição sem variável");
        return;
    }
    if (no->assign.expr == NULL) {
        gen_erro(c, no->lineno, "atribuição a '%s' sem valor", nome);
        return;
    }

    v = busca_variavel(c, nome);
    if (v == NULL && busca_funcao(c, nome) != NULL) {
        gen_erro(c, no->lineno, "não é possível atribuir à função '%s'", nome);
        return;
    }
    if (no->assign.op != AST_ASG_ASSIGN &&
        ((v && tipo_de_nome(v->tipo, v->is_array) == TE_STRING) ||
         tipo_expr(c, no->assign.expr) == TE_STRING)) {
        gen_erro(c, no->lineno, "'%s' com String ainda não é suportado", op);
        return;
    }

    emitir(c, 0, "%s %s ", nome, op);
    gen_expr_completa(c, no->assign.expr, nome, 0);
}

/* Comando formado só por uma expressão: "f(x);", "i++;". */
static void gen_expr_stmt(Ctx *c, const ASTNode *no, int nivel)
{
    const ASTNode *e = no->expr_stmt.expr;

    if (e == NULL) {
        gen_erro(c, no->lineno, "comando sem expressão");
        return;
    }
    if (e->type != NODE_CALL &&
        !(e->type == NODE_UNARY_OP &&
          (e->unary_op.op == AST_UN_INC || e->unary_op.op == AST_UN_DEC))) {
        gen_erro(c, no->lineno, "expressão usada como comando precisa ser uma "
                 "chamada, ++ ou --");
        return;
    }
    emitir(c, nivel, "");
    gen_expr_completa(c, e, NULL, 1);
    emitir(c, 0, ";\n");
}

/* --------------------------- declarações --------------------------- */

static void gen_var_decl(Ctx *c, const ASTNode *no, int nivel)
{
    const char *tipo_c;

    if (!nome_valido(c, no->var_decl.name, no->lineno, "variável"))
        return;
    tipo_c = tipo_c_de(c, no->var_decl.type_name, no->lineno);
    if (tipo_c == NULL)
        return;
    if (strcmp(tipo_c, "void") == 0) {
        gen_erro(c, no->lineno, "variável '%s' não pode ser void",
                 no->var_decl.name);
        return;
    }

    emitir(c, nivel, "");
    emitir_declarador(c, tipo_c, 0, no->var_decl.name);
    if (no->var_decl.init != NULL) {
        emitir(c, 0, " = ");
        gen_expr_completa(c, no->var_decl.init, NULL, 0);
    }
    emitir(c, 0, ";\n");

    /* Só fica visível depois do inicializador, como em Java. */
    declarar_visivel(c, no->var_decl.name, no->var_decl.type_name, 0);
}

static void gen_param(Ctx *c, const ASTNode *no)
{
    const char *tipo_c;

    if (no->param.type_name == NULL || no->param.type_name[0] == '\0') {
        gen_erro(c, no->lineno, "parâmetro sem tipo");
        return;
    }
    if (!nome_valido(c, no->param.name, no->lineno, "parâmetro"))
        return;
    tipo_c = tipo_c_de(c, no->param.type_name, no->lineno);
    if (tipo_c == NULL)
        return;
    if (strcmp(tipo_c, "void") == 0) {
        gen_erro(c, no->lineno, "parâmetro '%s' não pode ser void",
                 no->param.name);
        return;
    }
    emitir_declarador(c, tipo_c, no->param.is_array, no->param.name);
}

static void gen_params(Ctx *c, const ASTNode *params)
{
    const ASTNode *p;

    if (params == NULL) {
        emitir(c, 0, "void");
        return;
    }
    for (p = params; p != NULL && !c->erro; p = p->next) {
        if (p->type != NODE_PARAM) {
            gen_erro(c, p->lineno, "esperado um parâmetro");
            return;
        }
        if (p != params)
            emitir(c, 0, ", ");
        gen_param(c, p);
    }
}

/* É o `public static void main(String[] args)` do Java? */
static int eh_main_java(const ASTNode *f)
{
    const ASTNode *p = f->func_decl.params;

    return f->func_decl.name != NULL &&
           strcmp(f->func_decl.name, "main") == 0 &&
           f->func_decl.return_type != NULL &&
           strcmp(f->func_decl.return_type, "void") == 0 &&
           p != NULL && p->next == NULL && p->type == NODE_PARAM &&
           p->param.is_array && p->param.type_name != NULL &&
           strcmp(p->param.type_name, "String") == 0;
}

/* Valida o cabeçalho e devolve o tipo de retorno em C (ou NULL). */
static const char *cabecalho_funcao(Ctx *c, const ASTNode *f)
{
    if (!nome_valido(c, f->func_decl.name, f->lineno, "função"))
        return NULL;
    return tipo_c_de(c, f->func_decl.return_type, f->lineno);
}

static void gen_prototipo(Ctx *c, const ASTNode *f)
{
    const char *ret = cabecalho_funcao(c, f);

    if (ret == NULL)
        return;
    emitir_declarador(c, ret, 0, f->func_decl.name);
    emitir(c, 0, "(");
    gen_params(c, f->func_decl.params);
    emitir(c, 0, ");\n");
}

static void gen_func_decl(Ctx *c, const ASTNode *f, int nivel)
{
    const char *ret = cabecalho_funcao(c, f);
    const ASTNode *p;
    int main_java = 0;
    int ret_void_ant = c->retorno_void;
    int em_main_ant = c->em_main;
    size_t escopo = c->n_visiveis;

    if (ret == NULL)
        return;
    if (f->func_decl.body == NULL || f->func_decl.body->type != NODE_BLOCK) {
        gen_erro(c, f->lineno, "função '%s' sem corpo", f->func_decl.name);
        return;
    }

    if (strcmp(f->func_decl.name, "main") == 0) {
        /* Decisão desta versão: só aceitamos main(String[] args), gerada
         * como int main(void). Os args são ignorados. */
        if (!eh_main_java(f)) {
            gen_erro(c, f->lineno,
                     "assinatura de main não suportada (use "
                     "public static void main(String[] args))");
            return;
        }
        if (c->tem_main) {
            gen_erro(c, f->lineno, "mais de uma função main");
            return;
        }
        c->tem_main = 1;
        main_java = 1;
    }

    c->em_main = main_java;
    c->retorno_void = (strcmp(ret, "void") == 0) && !main_java;

    if (main_java)
        emitir(c, nivel, "int main(void)\n");
    else {
        emitir(c, nivel, "");
        emitir_declarador(c, ret, 0, f->func_decl.name);
        emitir(c, 0, "(");
        gen_params(c, f->func_decl.params);
        emitir(c, 0, ")\n");
        for (p = f->func_decl.params; p != NULL; p = p->next)
            declarar_visivel(c, p->param.name, p->param.type_name,
                             p->param.is_array);
    }
    emitir(c, nivel, "{\n");
    gen_lista(c, f->func_decl.body->block.statements, nivel + 1);
    if (main_java)
        emitir(c, nivel + 1, "return 0;\n");
    emitir(c, nivel, "}\n");

    c->n_visiveis = escopo;
    c->retorno_void = ret_void_ant;
    c->em_main = em_main_ant;
}

static void gen_return(Ctx *c, const ASTNode *no, int nivel)
{
    const ASTNode *e = no->return_stmt.expr;

    if (c->em_main) {
        /* main do Java é void; a do C devolve int. */
        if (e != NULL) {
            gen_erro(c, no->lineno, "main não pode retornar valor");
            return;
        }
        emitir(c, nivel, "return 0;\n");
        return;
    }
    if (c->retorno_void && e != NULL) {
        gen_erro(c, no->lineno, "função void não pode retornar valor");
        return;
    }
    if (!c->retorno_void && e == NULL) {
        gen_erro(c, no->lineno, "return sem valor em função não-void");
        return;
    }

    if (e == NULL) {
        emitir(c, nivel, "return;\n");
    } else {
        emitir(c, nivel, "return ");
        gen_expr_completa(c, e, NULL, 0);
        emitir(c, 0, ";\n");
    }
}

/* --------------------------- classe -------------------------------- */

/* Em C, variável global só aceita inicializador constante: literais,
 * combinados ou não por operadores (ex.: -1, 60 * 60, !true). Variáveis,
 * chamadas, ++ e -- ficam de fora. */
static int eh_constante(const ASTNode *e)
{
    if (e == NULL)
        return 0;
    switch (e->type) {
    case NODE_LITERAL_INT:
    case NODE_LITERAL_LONG:
    case NODE_LITERAL_DOUBLE:
    case NODE_LITERAL_FLOAT:
    case NODE_LITERAL_CHAR:
    case NODE_LITERAL_STRING:
    case NODE_LITERAL_BOOL:
        return 1;
    case NODE_UNARY_OP:
        return (e->unary_op.op == AST_UN_NEG || e->unary_op.op == AST_UN_NOT) &&
               eh_constante(e->unary_op.operand);
    case NODE_BIN_OP:
        return eh_constante(e->bin_op.left) && eh_constante(e->bin_op.right);
    default:
        return 0;
    }
}

/* Nomes de campos e funções dividem o escopo global do C. */
static int nome_repetido(Ctx *c, const ASTNode *membros)
{
    const ASTNode *a, *b;
    const char *na, *nb;

    for (a = membros; a != NULL; a = a->next) {
        na = (a->type == NODE_VAR_DECL) ? a->var_decl.name
           : (a->type == NODE_FUNC_DECL) ? a->func_decl.name : NULL;
        if (na == NULL)
            continue;
        for (b = a->next; b != NULL; b = b->next) {
            nb = (b->type == NODE_VAR_DECL) ? b->var_decl.name
               : (b->type == NODE_FUNC_DECL) ? b->func_decl.name : NULL;
            if (nb != NULL && strcmp(na, nb) == 0) {
                gen_erro(c, b->lineno,
                         "nome '%s' repetido (C não tem sobrecarga)", nb);
                return 1;
            }
        }
    }
    return 0;
}

/* Decisão da equipe (primeira versão): campos viram variáveis globais e
 * métodos viram funções. Ordem no C: campos, protótipos, definições. */
static void gen_class(Ctx *c, const ASTNode *no, int nivel)
{
    const ASTNode *m;
    int ha_campos = 0, ha_funcoes = 0;

    if (no->class_decl.name == NULL || no->class_decl.name[0] == '\0') {
        gen_erro(c, no->lineno, "classe sem nome");
        return;
    }
    emitir(c, nivel, "/* Classe %s */\n", no->class_decl.name);
    c->membros = no->class_decl.members;

    if (nome_repetido(c, no->class_decl.members))
        return;

    for (m = no->class_decl.members; m != NULL && !c->erro; m = m->next) {
        if (m->type == NODE_VAR_DECL) {
            if (m->var_decl.init != NULL && !eh_constante(m->var_decl.init)) {
                gen_erro(c, m->lineno, "campo '%s': inicializador de variável "
                         "global precisa ser constante (só literais e "
                         "operadores)",
                         m->var_decl.name ? m->var_decl.name : "");
                break;
            }
            gen_var_decl(c, m, nivel);
            ha_campos = 1;
        } else if (m->type == NODE_FUNC_DECL) {
            ha_funcoes = 1;
        } else {
            gen_erro(c, m->lineno, "membro de classe não suportado");
        }
    }

    if (ha_funcoes) {
        int protos = 0;

        for (m = no->class_decl.members; m != NULL && !c->erro; m = m->next) {
            if (m->type != NODE_FUNC_DECL || eh_main_java(m))
                continue;
            if (!protos) {
                if (ha_campos)
                    emitir(c, 0, "\n");
                protos = 1;
            }
            emitir(c, nivel, "");
            gen_prototipo(c, m);
        }
        for (m = no->class_decl.members; m != NULL && !c->erro; m = m->next) {
            if (m->type != NODE_FUNC_DECL)
                continue;
            emitir(c, 0, "\n");
            gen_func_decl(c, m, nivel);
        }
    }
}

/* --------------------------- percurso ------------------------------ */

static void gen_lista(Ctx *c, const ASTNode *lista, int nivel)
{
    const ASTNode *n;

    for (n = lista; n != NULL && !c->erro; n = n->next)
        gen_no(c, n, nivel);
}

static void gen_no(Ctx *c, const ASTNode *no, int nivel)
{
    if (c->erro)
        return;

    if (no == NULL) {
        gen_erro(c, 0, "nó de AST nulo");
        return;
    }

    switch (no->type) {

    case NODE_PROGRAM:
        gen_lista(c, no->program.body, nivel);
        /* Sem main no Java, geramos uma vazia para o C continuar válido. */
        if (!c->erro && !c->tem_main) {
            emitir(c, 0, "\nint main(void)\n{\n");
            emitir(c, 1, "return 0;\n");
            emitir(c, 0, "}\n");
        }
        break;

    case NODE_CLASS:
        gen_class(c, no, nivel);
        break;

    case NODE_VAR_DECL:
        gen_var_decl(c, no, nivel);
        break;

    case NODE_FUNC_DECL:
        gen_func_decl(c, no, nivel);
        break;

    case NODE_BLOCK: {
        size_t escopo = c->n_visiveis;

        emitir(c, nivel, "{\n");
        gen_lista(c, no->block.statements, nivel + 1);
        emitir(c, nivel, "}\n");
        c->n_visiveis = escopo;
        break;
    }

    case NODE_RETURN:
        gen_return(c, no, nivel);
        break;

    case NODE_ASSIGN:
        emitir(c, nivel, "");
        gen_atribuicao(c, no);
        emitir(c, 0, ";\n");
        break;

    case NODE_EXPR_STMT:
        gen_expr_stmt(c, no, nivel);
        break;

    default:
        /* Valor fora do enum NodeType: AST corrompida ou versão errada. */
        if ((int)no->type < (int)NODE_PROGRAM ||
            (int)no->type >= (int)NODE_COUNT) {
            gen_erro(c, no->lineno, "tipo de nó de AST desconhecido");
            break;
        }
        /* Nó válido, mas ainda não implementado: falha clara, nunca
         * comportamento indefinido. As próximas issues adicionam um
         * `case` por nó aqui. */
        gen_erro(c, no->lineno, "nó de AST não suportado");
        break;
    }
}

/* --------------------------- API pública --------------------------- */

int codegen_gerar(const ASTNode *raiz, FILE *saida)
{
    Ctx c;

    memset(&c, 0, sizeof c);
    c.saida = saida;

    if (raiz == NULL) {
        gen_erro(&c, 0, "AST nula");
        return CODEGEN_ERRO;
    }
    if (saida == NULL) {
        gen_erro(&c, 0, "fluxo de saída nulo");
        return CODEGEN_ERRO;
    }
    if (raiz->type != NODE_PROGRAM) {
        gen_erro(&c, raiz->lineno, "a raiz da AST deve ser NODE_PROGRAM");
        return CODEGEN_ERRO;
    }

    emitir(&c, 0, "/* Gerado automaticamente pelo compilador J2C */\n");
    emitir(&c, 0, "#include <stdio.h>\n");
    emitir(&c, 0, "#include <stdlib.h>\n");
    emitir(&c, 0, "#include <stdbool.h>\n\n");

    gen_no(&c, raiz, 0);
    free(c.visiveis);

    if (!c.erro && fflush(saida) == EOF)
        gen_erro(&c, 0, "impossível escrever no arquivo de saída");

    return c.erro ? CODEGEN_ERRO : CODEGEN_OK;
}

int codegen_gerar_arquivo(const ASTNode *raiz, const char *caminho)
{
    size_t tam;
    char *tmp;
    FILE *f;
    int rc;

    if (caminho == NULL) {
        fprintf(stderr, "Erro de geração de código: caminho de saída nulo\n");
        return CODEGEN_ERRO;
    }

    tam = strlen(caminho) + 5; /* ".tmp" + '\0' */
    tmp = malloc(tam);
    if (tmp == NULL) {
        fprintf(stderr, "Erro de geração de código: memória insuficiente\n");
        return CODEGEN_ERRO;
    }
    snprintf(tmp, tam, "%s.tmp", caminho);

    /* "wx": falha se o .tmp já existir, para nunca sobrescrever um arquivo
     * que não é nosso. */
    f = fopen(tmp, "wx");
    if (f == NULL) {
        fprintf(stderr, "Erro de geração de código: não foi possível criar "
                        "'%s' (já existe ou sem permissão)\n", tmp);
        free(tmp);
        return CODEGEN_ERRO;
    }

    rc = codegen_gerar(raiz, f);

    if (fclose(f) == EOF && rc == CODEGEN_OK) {
        fprintf(stderr, "Erro de geração de código: "
                        "impossível escrever no arquivo de saída\n");
        rc = CODEGEN_ERRO;
    }

    if (rc == CODEGEN_OK && rename(tmp, caminho) != 0) {
        /* No Windows o rename falha se o destino já existe: remove e
         * tenta de novo. */
        remove(caminho);
        if (rename(tmp, caminho) != 0) {
            fprintf(stderr, "Erro de geração de código: "
                            "não foi possível finalizar o arquivo de saída\n");
            rc = CODEGEN_ERRO;
        }
    }
    if (rc != CODEGEN_OK)
        remove(tmp);

    free(tmp);
    return rc;
}
