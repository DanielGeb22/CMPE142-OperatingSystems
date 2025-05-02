#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 10

int unsorted[SIZE] = {7, 12, 19, 3, 18, 4, 2, 6, 15, 8};
int sorted[SIZE];

typedef struct {
    int start;
    int end;
} parameters;

// Function to sort a subarray using bubble sort
void *sort_subarray(void *args) {
    parameters *p = (parameters *)args;
    for (int i = p->start; i < p->end; i++) {
        for (int j = p->start; j < p->end - (i - p->start) - 1; j++) {
            if (unsorted[j] > unsorted[j + 1]) {
                int temp = unsorted[j];
                unsorted[j] = unsorted[j + 1];
                unsorted[j + 1] = temp;
            }
        }
    }
    pthread_exit(0);
}

// Function to merge the two sorted subarrays
void *merge_subarrays(void *args) {
    int i = 0;                  // index for left half
    int j = SIZE / 2;           // index for right half
    int k = 0;                  // index for sorted array

    while (i < SIZE / 2 && j < SIZE) {
        if (unsorted[i] < unsorted[j]) {
            sorted[k++] = unsorted[i++];
        } else {
            sorted[k++] = unsorted[j++];
        }
    }

    while (i < SIZE / 2) {
        sorted[k++] = unsorted[i++];
    }

    while (j < SIZE) {
        sorted[k++] = unsorted[j++];
    }

    pthread_exit(0);
}

int main() {
    pthread_t tid1, tid2, tid3;

    parameters *p1 = (parameters *)malloc(sizeof(parameters));
    parameters *p2 = (parameters *)malloc(sizeof(parameters));

    p1->start = 0;
    p1->end = SIZE / 2;

    p2->start = SIZE / 2;
    p2->end = SIZE;

    // Create sorting threads
    pthread_create(&tid1, NULL, sort_subarray, p1);
    pthread_create(&tid2, NULL, sort_subarray, p2);

    // Wait for both sorting threads to finish
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // Create merge thread
    pthread_create(&tid3, NULL, merge_subarrays, NULL);

    // Wait for merge thread to finish
    pthread_join(tid3, NULL);

    // Print the sorted array
    printf("Sorted array: ");
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", sorted[i]);
    }
    printf("\n");

    // Free dynamically allocated memory
    free(p1);
    free(p2);

    return 0;
}