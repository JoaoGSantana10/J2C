public class Completo {
    private static int total = 0;

    public static int soma(int a, int b) {
        return a + b * 2;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int n = sc.nextInt();
        boolean ativo = !false;
        if (n > 0 && ativo) {
            total += soma(n, 3);
        } else {
            total = -1;
        }
        while (n != 0) {
            n--;
        }
        do {
            ++n;
        } while (n < 10);
        for (int i = 0; i < n; i++) {
            if (i == 2) continue;
            System.out.println("Ola");
        }
        switch (n) {
            case 1:
                break;
            default:
                total -= 'a';
        }
        for (;;) {
            break;
        }
    }
}
