public class Expressoes {
    static int contador = 0;
    static long limite = 60L * 60L;
    static double fator = -1.5;

    static int calcular(int x, int y) {
        return x * y;
    }

    static void registrar(int valor) {
        contador += valor;
    }

    static int exemplo(int a, int b, int c) {
        int resultado = a + b * c;
        resultado += 2;
        resultado++;
        registrar(calcular(a, b + 1));
        return resultado;
    }

    static boolean intervalo(int a, int b, int c) {
        return a <= b && b < c || !(a != b) && c >= 0;
    }

    static double misto(int i, double d) {
        int j = i++ + 1;
        return j / 2.0 + d * -fator;
    }

    public static void main(String[] args) {
        char letra = 'a';
        String texto = "J2C\tok";
        float taxa = 2.5f;
        boolean pronto = true;
        int n = 10 % 3;
        n -= 1;
        --n;
        registrar(n);
    }
}
