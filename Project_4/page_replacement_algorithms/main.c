#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "page_replacement_algorithms.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_frames>\n", argv[0]);
        return 1;
    }

    // Read number of frames
    int num_frames = atoi(argv[1]);
    if (num_frames <= 0) {
        fprintf(stderr, "Error: num_frames must be > 0\n");
        return 1;
    }

    // Read optional length of reference string
    int N = (argc > 2) ? atoi(argv[2]) : 20;
    if (N <= 0) {
        N = 20;
    }

    // Generate random reference string ranging from 0 to 9
    int *refs = malloc(N * sizeof(int));
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        refs[i] = rand() % 10;
    }

    // Print the reference string
    printf("Reference String (%d refs):\n", N);
    for (int i = 0; i < N; i++) {
        printf("%d ", refs[i]);
    }
    printf("\n\n");

    // Run page replacement algorithms then print their num of page faults
    int faultsFIFO = FIFO(num_frames, refs, N);
    int faultsLRU = LRU(num_frames, refs, N);
    int faultsOPT = OPT(num_frames, refs, N);

    printf("Page frames: %d\n", num_frames);
    printf("FIFO faults: %d\n", faultsFIFO);
    printf("LRU faults: %d\n", faultsLRU);
    printf("OPT faults: %d\n", faultsOPT);

    free(refs);

    return 0;
}