#include <stdio.h>
#include <omp.h>

int main(void) {

    double avg = 2;

    #pragma omp parallel for reduction(+:avg)
    for (int i = 0; i <30; i++) {
       avg+= 5;
    }
    printf("avg:%f\n",(avg/2));
    return 0;


    
}