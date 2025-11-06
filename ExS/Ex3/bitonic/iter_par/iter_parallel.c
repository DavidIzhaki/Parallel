#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "thr_pool.h"

#define N 32
#define MAX_THREADS 16

#define COLOR_RESET   "\033[0m"
#define COLOR_DIM     "\033[2m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"

typedef struct {
    int *arr;
    int i;
    int j;
    int k;
} task_data_t;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void printArrayRangeHighlighted(int *array, int start, int end, int highlight1, int highlight2, const char *baseColor) {
    printf("%s[", baseColor);
    for (int i = start; i <= end; i++) {
        if (i == highlight1 || i == highlight2) {
            printf("\033[33m%d\033[0m ", array[i]);  // YELLOW
        } else {
            printf("%s%d%s ", baseColor, array[i], COLOR_RESET);
        }
    }
    printf("]%s\n", COLOR_RESET);
}

void printArrayRange(int *array, int start, int end, const char *color) {
    printf("%s[", color);
    for (int i = start; i <= end; i++) {
        printf("%d ", array[i]);
    }
    printf("]%s\n", COLOR_RESET);
}

void *bitonic_task(void *arg) {
    task_data_t *data = (task_data_t *)arg;
    int ixj = data->i ^ data->j;
    pthread_t tid = pthread_self();

    if (ixj > data->i) {
        int blockStart = (data->i / data->k) * data->k;
        int blockEnd = blockStart + data->k - 1;
        const char *color = (data->i & data->k) == 0 ? COLOR_GREEN : COLOR_RED;
        const char *dir = (data->i & data->k) == 0 ? "ASCENDING" : "DESCENDING";

        pthread_mutex_lock(&print_mutex);

        printf("\n%s[Thread %lu] Subarray (%s) BEFORE:%s ", COLOR_DIM, tid, dir, COLOR_RESET);
        printArrayRangeHighlighted(data->arr, blockStart, blockEnd, data->i, ixj, COLOR_DIM);

        int shouldSwap = ((data->i & data->k) == 0 && data->arr[data->i] > data->arr[ixj]) ||
                         ((data->i & data->k) != 0 && data->arr[data->i] < data->arr[ixj]);

        if (shouldSwap) {
            swap(&data->arr[data->i], &data->arr[ixj]);
        }

        printf("%s[Thread %lu] Subarray (%s) AFTER:%s  ", color, tid, dir, COLOR_RESET);
        printArrayRange(data->arr, blockStart, blockEnd, color);
        pthread_mutex_unlock(&print_mutex);
    }
    free(data);
    return NULL;
}

void printFullArray(int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

int main() {
    int arr[N] = {
            10, 30, 11, 20, 4, 33, 2, 1, 99, 54, 3, 76, 23, 19, 15, 8,
            55, 43, 21, 7, 60, 12, 5, 88, 45, 17, 25, 38, 6, 70, 9, 0
    };
    thr_pool_t *pool = thr_pool_create(8, MAX_THREADS, 5, NULL);

    printf("Original array:\n");
    printFullArray(arr, N);

    for (int k = 2; k <= N; k *= 2) {
        printf("\n=== Sequence size k = %d ===\n", k);
        for (int j = k / 2; j > 0; j /= 2) {
            for (int i = 0; i < N; i++) {
                task_data_t *data = malloc(sizeof(task_data_t));
                data->arr = arr;
                data->i = i;
                data->j = j;
                data->k = k;
                thr_pool_queue(pool, bitonic_task, data);
            }
            thr_pool_wait(pool); // Wait for all tasks at this stage
        }
    }

    thr_pool_destroy(pool);

    printf("\nSorted array:\n");
    printFullArray(arr, N);

    return 0;
}
