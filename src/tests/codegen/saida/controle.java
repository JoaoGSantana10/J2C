public class Controle {
    static int fatorial(int n) {
        int r = 1;
        for (int i = 2; i <= n; i++) {
            r = r * i;
        }
        return r;
    }

    static boolean par(int n) {
        return n % 2 == 0;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int n = sc.nextInt();

        for (int i = 1; i <= n; i++) {
            if (i == 3)
                continue;
            if (par(i)) {
                System.out.println(i);
            } else {
                System.out.print("impar ");
                System.out.println(i);
            }
        }

        int k = n;
        while (k > 0) {
            k -= 2;
            if (k == 1)
                break;
        }
        System.out.println(k);

        int digitos = 0;
        int x = 1000;
        do {
            digitos++;
            x = x / 10;
        } while (x != 0);
        System.out.println(digitos);

        switch (n) {
            case 4:
            case 5:
                System.out.println("quatro ou cinco");
                break;
            default:
                System.out.println("outro");
        }

        System.out.println(fatorial(n));
    }
}
