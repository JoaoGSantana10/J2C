public class chamadas_metodos {
    void imprimir(int n) {
        System.out.println(n);
        proximo(n);
    }
    void proximo(int n) {
        imprimir(n);
    }
}
