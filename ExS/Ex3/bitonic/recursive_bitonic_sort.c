#include <stdio.h>

#define ASCENDING 1
#define DESCENDING 0

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void merge(int *array,int n, int direction){
    if(n <= 1){
        return;
    }
    int mid = n / 2;

    if(direction == ASCENDING){
        for (int i = 0; i < mid; i++) {
            //compre and swap
            if (array[i] > array[n/2 + i]){
                swap(&array[i], &array[i + mid]);
            }
        }

    } else{ //descending case
        for (int i = 0; i < mid; i++) {
            //compre and swap
            if (array[i] < array[n/2 + i]){
                swap(&array[i], &array[i + mid]);
            }
        }
    }
    merge(array,n/2, direction);
    merge(array + n/2, n/2, direction);
    return;
}

void bitonicSort(int *array, int n, int direction) {
    if(n <= 1){
        return;
    }

    int mid = n / 2;
    bitonicSort(array, mid, ASCENDING);
    bitonicSort(array + mid, mid, DESCENDING);

    merge(array,n, direction);

}

void printArray(int *array, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int main() {
    int array[] = {10, 30, 11, 20, 4, 6, 7, 2, 8, 1, 15, 12, 14, 3, 5, 9};
    int n = sizeof(array) / sizeof(array[0]);

    printf("Original array:\n");
    printArray(array, n);

    bitonicSort(array, n, ASCENDING);

    printf("Sorted array:\n");
    printArray(array, n);

    return 0;
}
