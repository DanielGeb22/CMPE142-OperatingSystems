#include <stdlib.h>
#include <stdio.h>
#include "page_replacement_algorithms.h"

int is_in_frames(int frames[], int F, int page) {
    for (int i = 0; i < F; i++) {
        if (frames[i] == page) {
            return i;
        }
    }
    return -1;
}

int find_empty(int frames[], int F) {
    for (int i = 0; i < F; i++) {
        if (frames[i] == -1) {
            return i;
        }
    }
    return -1;
}

// Page Replacement Algorithms
int FIFO(int F, int refs[], int N) {
    int *frames = malloc(F * sizeof(int));
    for (int i = 0; i < F; i++)  {
        frames[i] = -1;
    }

    int pageFaults = 0, fifo_idx = 0;
    for (int i = 0; i < N; i++) {
        int p = refs[i];
        if (is_in_frames(frames, F, p) < 0) {
            frames[fifo_idx] = p;
            fifo_idx = (fifo_idx + 1) % F;
            pageFaults++;
        }
    }
    free(frames);
    return pageFaults;
}

int LRU(int F, int refs[], int N) {
    int *frames    = malloc(F * sizeof(int));
    int *last_used = malloc(F * sizeof(int));
    for (int i = 0; i < F; i++) {
        frames[i] = -1;
        last_used[i] = 0;
    }

    int pageFaults = 0, timer = 0;
    for (int i = 0; i < N; i++) {
        int p = refs[i];
        int idx = is_in_frames(frames, F, p);
        if (idx >= 0) {
            /* Hit: update recency */
            last_used[idx] = timer++;
        } 
        else {
            /* Fault */
            pageFaults++;
            int empty = find_empty(frames, F);
            if (empty >= 0) {
                frames[empty] = p;
                last_used[empty] = timer++;
            }
            else {
                /* Evict least-recently-used */
                int lru = 0;
                for (int j = 1; j < F; j++) {
                    if (last_used[j] < last_used[lru]) {
                        lru = j;
                    }
                }
                frames[lru] = p;
                last_used[lru] = timer++;                   
            }
        }
    }

    free(frames);
    free(last_used);
    return pageFaults;
}

int OPT(int F, int refs[], int N) {
    int *frames = malloc(F * sizeof(int));
    for (int i = 0; i < F; i++) frames[i] = -1;

    int pageFaults = 0;
    for (int i = 0; i < N; i++) {
        int p = refs[i];
        if (is_in_frames(frames, F, p) >= 0)
            continue;  // hit

        /* fault */
        pageFaults++;
        int empty = find_empty(frames, F);
        if (empty >= 0) {
            frames[empty] = p;
        } else {
            int far_idx = 0;
            int farthest = -1;
            for (int j = 0; j < F; j++) {
                int next_use = N;  // assume “never used again”
                for (int k = i + 1; k < N; k++) {
                    if (refs[k] == frames[j]) {
                        next_use = k;
                        break;
                    }
                }
                if (next_use > farthest) {
                    farthest = next_use;
                    far_idx = j;
                }
            }
            frames[far_idx] = p;
        }
    }

    free(frames);
    return pageFaults;
}