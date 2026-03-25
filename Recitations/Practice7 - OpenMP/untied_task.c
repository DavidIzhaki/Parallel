#include <stdio.h>
#include <omp.h>

void busy_work(const char *label) {
    volatile double x = 0.0;
    for (long i = 0; i < 200000000; i++) {
        x += i * 0.0000001;
    }
    printf("%s: child done on thread %d\n",
           label, omp_get_thread_num());
}

int main(void) {
    omp_set_num_threads(4);

    #pragma omp parallel
    {
        #pragma omp single
        {
             int t0 = omp_get_thread_num();
                printf("Single: start on thread %d\n", t0);
            // ---------- TIED parent task (default) ----------
            
            #pragma omp task  // TIED parent
            {
                int t0 = omp_get_thread_num();
                printf("TIED parent: start on thread %d\n", t0);

                #pragma omp task // CHILD task
                {
                    busy_work("TIED parent");
                }

                #pragma omp taskwait  // parent may be suspended here

                int t1 = omp_get_thread_num();
                printf("TIED parent: after taskwait on thread %d "
                       "(must be SAME)\n", t1);
            }

            // ---------- UNTIED parent task ----------
            #pragma omp task untied // UNTIED parent
            {
                int t0 = omp_get_thread_num();
                printf("UNTIED parent: start on thread %d\n", t0);

                #pragma omp task // CHILD task
                {
                    busy_work("UNTIED parent");
                }

                #pragma omp taskwait  // parent may be suspended here

                int t1 = omp_get_thread_num();
                printf("UNTIED parent: after taskwait on thread %d "
                       "(may be DIFFERENT)\n", t1);
            }
        }
    }

    return 0;
}
