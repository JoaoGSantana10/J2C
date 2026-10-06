public class test_continue_em_laco {

    int test(){
        int x = 0;
        while(x < 10){
            x++;
            if(x == 5){
                continue;
            }
        }
        return 0;
    }
}
