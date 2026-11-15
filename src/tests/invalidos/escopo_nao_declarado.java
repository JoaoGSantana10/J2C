public class escopo_nao_declarado {
    public static void main(String[] args) {
        int x = 1;
        {
            int y = 2;
        }
        y = 3;
}
