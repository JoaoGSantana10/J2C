int testeVariaveis() {
    int a = 10;
    double b = 20.5;
    boolean flag = true;
    char c = 'x';

    a += 5;
    b -= 2.0;
    a++;

    boolean res = (a > 10) && (b <= 30.0) || !flag;
    return a;
}