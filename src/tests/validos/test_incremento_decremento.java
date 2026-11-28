public class test_incremento_decremento {

    int test(){
        int i = 5;
        i++;
        ++i;
        i--;
        --i;
        for(int j = 10; j > 0; j--){
            i++;
        }
        for(int k = 0; k < 3; ++k){
            --i;
        }
        return i;
    }
}
