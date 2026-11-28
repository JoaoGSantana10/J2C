public class test_continue_em_for {

    int test(){
        int soma = 0;
        for(int i = 0; i < 10; i++){
            if(i % 2 == 0){
                continue;
            }
            soma += i;
        }
        for(int j = 0; j < 3; j++){
            continue;
        }
        return soma;
    }
}
