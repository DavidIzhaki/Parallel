#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

typedef struct element {
    int value;
    struct element *next;
} element;

void Process(element *p) {
    // Fake work on the node
    printf("Processing value %d on thread %d\n",
           p->value, omp_get_thread_num());
}

void process_list_in_parallel(element *listHead) {

    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            element *p = listHead;

            while (p != NULL) {
                // Create one task per list node
                #pragma omp task firstprivate(p)
                {
                    // Each task gets its own copy of pointer p
                    Process(p);
                }

                // Only the single thread walks the list
                p = p->next;
            }

            // Wait until all tasks created by this single thread are done
            #pragma omp taskwait
        }
    }
}

int main(void) {
    // Build a simple linked list: 0 -> 1 -> ... -> 9
    element *head = NULL;
    for (int i = 9; i >= 0; --i) {
        element *node = malloc(sizeof(element));
        if (!node) {
            perror("malloc");
            return 1;
        }
        node->value = i;
        node->next = head;
        head = node;
    }

    omp_set_num_threads(4);  // choose how many threads you want

    printf("Processing list in parallel:\n");
    process_list_in_parallel(head);

    // Free the list
    element *p = head;
    while (p != NULL) {
        element *next = p->next;
        free(p);
        p = next;
    }

    return 0;
}
