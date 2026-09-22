int somar(int x, int y) {
    return x + y;
}

int multiplicar(int a, int b) {
    return a * b;
}

int executar() {
    int res = somar(10, 20);
    int total = multiplicar(res, 2);
    return total;
}