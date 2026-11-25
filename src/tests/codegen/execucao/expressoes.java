public class Calculos {
    static int base = 60 * 60;
    static int contador = 0;

    static int precedencia(int a, int b, int c) { return a + b * c; }
    static int agrupado(int a, int b, int c) { return (a + b) * c; }
    static int associatividade(int a, int b, int c) { return a - b - c; }
    static int divisoes(int a, int b, int c) { return a / b / c; }
    static int resto(int a, int b) { return a % b; }

    static int posfixo(int i) { int r = i++; return r * 10 + i; }
    static int prefixo(int i) { int r = ++i; return r * 10 + i; }
    static int decrementos(int i, int j) { int r = i-- - --j; return r * 100 + i * 10 + j; }
    static int compostos(int x) { x += 3; x -= 1; x++; return x; }
    static int negativos(int a) { return -a * -a - -(-a); }

    static boolean logico(int a, int b, int c) { return a < b && b < c || !(a != 0); }

    static long grande() { return 3000000000L * 2; }
    static double media(int a, int b) { return (a + b) / 2.0; }
    static int mediaInteira(int a, int b) { return (a + b) / 2; }
    static float metade(float x) { return x / 2f; }
    static int codigo(char ch) { return ch + 1; }

    static int chamadas(int x, int y) { return precedencia(x, y + 1, 2) + agrupado(1, 1, x); }
    static int campo() { return base / 60; }
    static void conta(int n) { contador += n; }
    static int usaContador() { conta(2); conta(3); contador--; return contador; }

    public static void main(String[] args) {
    }
}
