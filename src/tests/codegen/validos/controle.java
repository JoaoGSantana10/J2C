public class Controle {
    static int somaAte(int n) {
        int s = 0;
        for (int i = 1; i <= n; i++) {
            s += i;
        }
        return s;
    }

    static int somaPares(int n) {
        int s = 0;
        for (int i = 0; i < n; i++) {
            if (i % 2 != 0)
                continue;
            s += i;
        }
        return s;
    }

    static int primeiroMultiplo(int n, int d) {
        int i = n;
        while (true) {
            if (i % d == 0) {
                break;
            }
            i++;
        }
        return i;
    }

    static int contaDigitos(int n) {
        int c = 0;
        do {
            c++;
            n = n / 10;
        } while (n != 0);
        return c;
    }

    static int sinal(int x) {
        if (x > 0) {
            return 1;
        } else if (x < 0) {
            return -1;
        } else {
            return 0;
        }
    }

    static int diasNoMes(int m) {
        switch (m) {
            case 2:
                return 28;
            case 4:
            case 6:
            case 9:
            case 11:
                return 30;
            default:
                return 31;
        }
    }

    static int pontos(char letra) {
        switch (letra) {
            case 'a': return 1;
            case 'b': return 2;
            default: return 0;
        }
    }

    static int acumula(int k) {
        int r = 0;
        switch (k) {
            case 1:
                r += 1;
            case 2:
                r += 10;
                break;
            case 3:
                r += 100;
        }
        return r;
    }

    static int comDeclaracao(int k) {
        switch (k) {
            case 1:
                int x = 5;
                return x * 2;
            default:
                return 0;
        }
    }

    static int aninhado(int n) {
        int t = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (j > i)
                    break;
                t++;
            }
        }
        return t;
    }

    static boolean primo(int n) {
        if (n < 2)
            return false;
        for (int d = 2; d * d <= n; d++) {
            if (n % d == 0)
                return false;
        }
        return true;
    }

    static int doisFors() {
        int s = 0;
        for (int i = 0; i < 3; i++) {
            s += i;
        }
        for (int i = 10; i < 12; i++) {
            s += i;
        }
        return s;
    }

    static int forSemInit() {
        int i = 0;
        for (; i < 5;) {
            i += 2;
        }
        return i;
    }

    public static void main(String[] args) {
        for (int i = 0; i < 3; i++) {
            if (i == 1)
                return;
        }
    }
}
