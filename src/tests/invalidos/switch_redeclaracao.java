public class switch_redeclaracao {
    void m(int op) {
        switch (op) {
            case 1:
                int a = 1;
                break;
            case 2:
                int a = 2;
                break;
        }
    }
}
