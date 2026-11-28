public class Leitura {
    static Scanner entrada = new Scanner(System.in);

    static int lerSoma() {
        int a = entrada.nextInt();
        int b = entrada.nextInt();
        return a + b;
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.println("Digite dois inteiros:");
        int soma = lerSoma();
        System.out.println(soma);
        double x = sc.nextDouble();
        System.out.println(x * 2);
        System.out.println(sc.nextInt() - 1);
        sc.close();
    }
}
