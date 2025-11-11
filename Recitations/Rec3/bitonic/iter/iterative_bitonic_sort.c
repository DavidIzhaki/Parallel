#include <stdio.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_DIM     "\033[2m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void printArrayRangeHighlighted(int *array, int start, int end, int highlight1, int highlight2, const char *baseColor) {
    printf("%s[", baseColor);
    for (int i = start; i <= end; i++) {
        if (i == highlight1 || i == highlight2) {
            printf("\033[33m%d\033[0m ", array[i]);  // YELLOW then RESET immediately
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

void bitonicSortIterative(int *arr, int n) {
    int i, j, k, l;

    for (k = 2; k <= n; k *= 2) {
        printf("\n=== Sequence size k = %d ===\n", k);

        for (j = k / 2; j > 0; j /= 2) {

            for (i = 0; i < n; i++) {
                l = i ^ j;
                if (l > i) {
                    int shouldSwap = ((i & k) == 0 && arr[i] > arr[l]) ||
                                     ((i & k) != 0 && arr[i] < arr[l]);

                    // Subarray info
                    int blockStart = (i / k) * k;
                    int blockEnd = blockStart + k - 1;
                    const char *color = (i & k) == 0 ? COLOR_GREEN : COLOR_RED;
                    const char *dir = (i & k) == 0 ? "ASCENDING" : "DESCENDING";

                    printf("\n%sSubarray (%s) BEFORE:%s ", COLOR_DIM, dir, COLOR_RESET);
                    printArrayRangeHighlighted(arr, blockStart, blockEnd, i, l, COLOR_DIM);

                    if (shouldSwap) {
                        swap(&arr[i], &arr[l]);
                    }

                    printf("%sSubarray (%s) AFTER:%s  ", color, dir, COLOR_RESET);
                    printArrayRange(arr, blockStart, blockEnd, color);
                }
            }
        }
    }
}

void printFullArray(int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

int main() {
    int arr[] = {
        10, 30, 11, 20, 4, 33, 2, 1, 99, 54, 3, 76, 23, 19, 15, 8,
        55, 43, 21, 7, 60, 12, 5, 88, 45, 17, 25, 38, 6, 70, 9, 0
    };
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("Original array:\n");
    printFullArray(arr, n);

    bitonicSortIterative(arr, n);

    printf("\nSorted array:\n");
    printFullArray(arr, n);

    return 0;
}
