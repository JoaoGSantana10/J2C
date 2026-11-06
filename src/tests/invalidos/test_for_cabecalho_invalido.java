public class test_for_cabecalho_invalido {
    int teste() {
        for (int i = 0 i < 10; i++) { // Erro: falta ';' entre a inicialização e a condição do for
            int a = 1;
        }
        return 0;
    }
}
