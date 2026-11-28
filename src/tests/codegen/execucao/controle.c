/* Driver do teste de execução de controle.java.
 * Chama as funções do C gerado e compara com o resultado que o Java dá
 * para as mesmas chamadas. Retorna 0 se tudo bate. */
#include <stdbool.h>
#include <stdio.h>

int somaAte(int n);
int somaPares(int n);
int primeiroMultiplo(int n, int d);
int contaDigitos(int n);
int sinal(int x);
int diasNoMes(int m);
int pontos(char letra);
int acumula(int k);
int comDeclaracao(int k);
int aninhado(int n);
bool primo(int n);
int doisFors(void);
int forSemInit(void);
int j2c_main(void);

static int falhas = 0;

#define CONFERE(expr, esperado) do {                                        \
    long long obtido_ = (long long)(expr);                                  \
    if (obtido_ != (long long)(esperado)) {                                 \
        printf("  FALHOU: %s = %lld (Java: %lld)\n", #expr, obtido_,        \
               (long long)(esperado));                                      \
        falhas++;                                                           \
    }                                                                       \
} while (0)

int main(void)
{
    CONFERE(somaAte(10), 55);
    CONFERE(somaAte(0), 0);                   /* for não executa      */
    CONFERE(somaPares(10), 20);               /* 0 + 2 + 4 + 6 + 8    */
    CONFERE(primeiroMultiplo(13, 5), 15);     /* while (true) + break */
    CONFERE(primeiroMultiplo(10, 5), 10);
    CONFERE(contaDigitos(12345), 5);
    CONFERE(contaDigitos(0), 1);              /* do-while roda 1 vez  */
    CONFERE(sinal(7), 1);
    CONFERE(sinal(-3), -1);
    CONFERE(sinal(0), 0);
    CONFERE(diasNoMes(2), 28);
    CONFERE(diasNoMes(9), 30);                /* cases empilhados     */
    CONFERE(diasNoMes(1), 31);                /* default              */
    CONFERE(pontos('a'), 1);
    CONFERE(pontos('b'), 2);
    CONFERE(pontos('z'), 0);
    CONFERE(acumula(1), 11);                  /* fallthrough 1 -> 2   */
    CONFERE(acumula(2), 10);
    CONFERE(acumula(3), 100);                 /* último case sem break */
    CONFERE(acumula(9), 0);                   /* sem default          */
    CONFERE(comDeclaracao(1), 10);
    CONFERE(comDeclaracao(2), 0);
    CONFERE(aninhado(4), 10);                 /* break sai só do for interno */
    CONFERE(primo(7), true);
    CONFERE(primo(9), false);
    CONFERE(primo(1), false);
    CONFERE(doisFors(), 24);                  /* 0+1+2 + 10+11        */
    CONFERE(forSemInit(), 6);
    CONFERE(j2c_main(), 0);                   /* return; -> return 0; */

    return falhas == 0 ? 0 : 1;
}
