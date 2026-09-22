int testeControle(int opcao) {
    int i = 0;
    int soma = 0;

    if (opcao == 1) {
        soma = 100;
    } else if (opcao == 2) {
        soma = 200;
    } else {
        soma = 0;
    }

    while (i < 10) {
        soma += i;
        i++;
    }

    do {
        soma -= 1;
    } while (soma > 150);

    for (int j = 0; j < 5; j++) {
        if (j == 3) {
            break;
        }
    }

    switch (opcao) {
        case 1:
            soma = 10;
            break;
        case 2:
            soma = 20;
            break;
        default:
            soma = 0;
    }

    return soma;
}