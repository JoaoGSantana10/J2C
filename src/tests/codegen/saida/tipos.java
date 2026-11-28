public class Tipos {
    static int dobro(int x) {
        return x * 2;
    }

    public static void main(String[] args) {
        int i = 7;
        long grande = 3000000000L * 2;
        double d = 2.5;
        float f = 1.5f;
        char letra = 'J';
        boolean ativo = i > 5;
        String nome = "J2C";
        System.out.println(i);
        System.out.println(i + 1);
        System.out.println(dobro(i));
        System.out.println(grande);
        System.out.println(d);
        System.out.println(f);
        System.out.println(letra);
        System.out.println(ativo);
        System.out.println(!ativo);
        System.out.println(nome);
        System.out.print("sem ");
        System.out.print("quebra");
        System.out.println();
        System.out.println("fim");
    }
}
