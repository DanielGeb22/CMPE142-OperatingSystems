#ifndef PAGE_REPLACEMENT_ALGORITHMS_H
#define PAGE_REPLACEMENT_ALGORITHMS_H

// Helper functions
int is_in_frames(int frames[], int F, int page);    // checks if page is in the frames
int find_empty(int frames[], int F);                // looks for empty frame

// Replacement algorithms
int FIFO(int F, int refs[], int N);
int LRU(int F, int refs[], int N);
int OPT(int F, int refs[], int N);

#endif