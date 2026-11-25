/* Driver do teste de execução de expressoes.java.
 * Chama as funções do C gerado e compara com o resultado que o Java dá
 * para as mesmas chamadas. Retorna 0 se tudo bate. */
#include <stdbool.h>
#include <stdio.h>

int precedencia(int a, int b, int c);
int agrupado(int a, int b, int c);
int associatividade(int a, int b, int c);
int divisoes(int a, int b, int c);
int resto(int a, int b);
int posfixo(int i);
int prefixo(int i);
int decrementos(int i, int j);
int compostos(int x);
int negativos(int a);
bool logico(int a, int b, int c);
long long grande(void);
double media(int a, int b);
int mediaInteira(int a, int b);
float metade(float x);
int codigo(char ch);
int chamadas(int x, int y);
int campo(void);
int usaContador(void);

static int falhas = 0;

#define CONFERE(expr, esperado) do {                                        \
    double obtido_ = (double)(expr);                                        \
    if (obtido_ != (double)(esperado)) {                                    \
        printf("  FALHOU: %s = %.17g (Java: %.17g)\n", #expr, obtido_,      \
               (double)(esperado));                                         \
        falhas++;                                                           \
    }                                                                       \
} while (0)

int main(void)
{
    CONFERE(precedencia(2, 3, 4), 14);        /* 2 + (3 * 4)        */
    CONFERE(agrupado(2, 3, 4), 20);           /* (2 + 3) * 4        */
    CONFERE(associatividade(10, 3, 2), 5);    /* (10 - 3) - 2       */
    CONFERE(divisoes(100, 5, 2), 10);         /* (100 / 5) / 2      */
    CONFERE(resto(-7, 2), -1);                /* Java trunca para 0 */
    CONFERE(resto(7, -2), 1);
    CONFERE(posfixo(5), 56);                  /* r = 5, i = 6       */
    CONFERE(prefixo(5), 66);                  /* r = 6, i = 6       */
    CONFERE(decrementos(5, 5), 144);          /* r = 5 - 4, i = 4, j = 4 */
    CONFERE(compostos(10), 13);
    CONFERE(negativos(3), 6);                 /* 9 - 3              */
    CONFERE(logico(1, 2, 3), true);
    CONFERE(logico(3, 2, 1), false);
    CONFERE(logico(0, 5, 1), true);           /* !(0 != 0)          */
    CONFERE(grande(), 6000000000LL);          /* não estoura: long  */
    CONFERE(media(3, 4), 3.5);
    CONFERE(mediaInteira(3, 4), 3);
    CONFERE(mediaInteira(-3, -4), -3);        /* -7 / 2 trunca para 0 */
    CONFERE(metade(5.0f), 2.5);
    CONFERE(codigo('A'), 66);
    CONFERE(chamadas(2, 3), 14);              /* 10 + 4             */
    CONFERE(campo(), 60);                     /* 3600 / 60          */
    CONFERE(usaContador(), 4);                /* 0 + 2 + 3 - 1      */

    return falhas == 0 ? 0 : 1;
}
