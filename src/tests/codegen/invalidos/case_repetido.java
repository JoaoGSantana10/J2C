public class CaseRepetido {
    static int f(int x) {
        switch (x) {
            case 1:
                return 1;
            case 1:
                return 2;
        }
        return 0;
    }
}
