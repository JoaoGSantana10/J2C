int soma(int a, int b) {
    int resultado = a + b;
    if (resultado > 10) {
        resultado = resultado * 2;
    } else {
        resultado = 0;
    }
    
    while (resultado < 50) {
        resultado += 5;
    }

    return resultado;
}