#include "allocator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    size_t start, size;
} Hole;

static Hole *holes = NULL;
static size_t hole_count = 0, hole_capacity = 0;

typedef struct {
    char pid[16];
    size_t start, size;
} Allocated_Block;

static Allocated_Block *blocks = NULL;
static size_t block_count = 0, block_capacity = 0;

static size_t total_mem = 0;


// Helper functions
static void resize_holes_array_capacity(void) {
    if (hole_count >= hole_capacity) {
        hole_capacity = hole_capacity ? hole_capacity * 2 : 4;
        holes = realloc(holes, hole_capacity * sizeof *holes);
        if (!holes) {
            perror("realloc holes");
            exit(EXIT_FAILURE);
        }
    }
}

static void resize_alloc_block_array_capacity(void) {
    if (block_count >= block_capacity) {
        block_capacity = block_capacity ? block_capacity * 2 : 4;
        blocks = realloc(blocks, block_capacity * sizeof *blocks);
        if (!blocks) {
            perror("realloc blocks");
            exit(EXIT_FAILURE);
        }
    }
}

static int first_fit_allocation(size_t size) {
    for (size_t i = 0; i < hole_count; i++) {
        if (holes[i].size >= size) {
            return (int)i;
        }
    }
    return -1;
}

static int best_fit_allocation(size_t size) {
    int best = -1;
    for (size_t i = 0; i < hole_count; i++) {
        if (holes[i].size >= size) {
            if (best < 0 || holes[i].size < holes[best].size) {
                best = (int)i;
            }
        }
    }
    return best;
}

static int worst_fit_allocation(size_t size) {
    int worst = -1;
    for (size_t i = 0; i < hole_count; i++) {
        if (holes[i].size >= size) {
            if (worst < 0 || holes[i].size > holes[worst].size) {
                worst = (int)i;
            }
        }
    }
    return worst;
}

// Compare function for quicksort (qsort) to sort holes by start address
static int compare_holes(const void *a, const void *b) {
    const Hole *ha = a;
    const Hole *hb = b;
    return (ha->start < hb->start) ? -1 : (ha->start > hb->start);
}

// Compare function for quicksort (qsort) to sort allocated blocks by start address
static int compare_blocks(const void *a, const void *b) {
    const Allocated_Block *ba = a;
    const Allocated_Block *bb = b;
    return (ba->start < bb->start) ? -1 : (ba->start > bb->start);
}

// Merge adjacent and overlapping free holes
static void merge_holes(void) {
    if (hole_count < 2) return;
    qsort(holes, hole_count, sizeof *holes, compare_holes);
    size_t write = 0;
    for (size_t i = 1; i < hole_count; i++) {
        size_t prev_end = holes[write].start + holes[write].size;
        size_t curr_start = holes[i].start;
        size_t curr_end = curr_start + holes[i].size;

        if (prev_end >= curr_start) {
            // Extend the previous hole to cover overlap/gap
            size_t base = holes[write].start;
            holes[write].size = (curr_end > base + holes[write].size) ? (curr_end - base) : holes[write].size;
        } else {
            holes[++write] = holes[i];
        }
    }
    hole_count = write + 1;
}


// Commands
static void RQ(char **toks, int n) {
    if (n != 4) {
        printf("RQ requires 3 arguments\n");
        return;
    }
    char *pid = toks[1];                        // process
    size_t sz = strtoull(toks[2], NULL, 10);    // amount of memory requested
    char alg = toks[3][0];                      // strategy (F, B, or W)

    // Determine allocation algorithm
    int idx = -1;
    switch (alg) {
        case 'F':
            idx = first_fit_allocation(sz);
            break;
        case 'B':
            idx = best_fit_allocation(sz);
            break;
        case 'W':
            idx = worst_fit_allocation(sz);
            break;
        default:
            printf("Error: Unknown algorithm %c\n", alg);
            return;
    }

    // No suitable hole, reject request
    if (idx < 0) {
        printf("Error: No available memory for %s\n", pid);
        return;
    }

    // Allocate
    size_t base = holes[idx].start;
    resize_alloc_block_array_capacity();
    strcpy(blocks[block_count].pid, pid);
    blocks[block_count].start = base;
    blocks[block_count].size = sz;
    block_count++;

    // Update list of free holes by shrinking or removing selected hole
    if (holes[idx].size > sz) {
        holes[idx].start += sz;
        holes[idx].size -= sz;
    }
    else {
        memmove(&holes[idx], &holes[idx+1], (hole_count - idx - 1) * sizeof *holes);
        hole_count--;
    }
}

static void RL(char **toks, int n) {
    if (n != 2) {
        printf("Error: RL requires 1 argument\n");
        return;
    }
    char *pid = toks[1];
    size_t i;

    // Find the allocation by process ID
    for (i = 0; i < block_count; i++) {
        if (strcmp(blocks[i].pid, pid) == 0) break;
    }
    if (i == block_count) {
        printf("Error: %s not found\n", pid);
        return;
    }

    // Add its region back to list of free holes
    resize_holes_array_capacity();
    holes[hole_count].start = blocks[i].start;
    holes[hole_count].size  = blocks[i].size;
    hole_count++;

    // Remove from allocated blocks array
    memmove(&blocks[i], &blocks[i+1],
            (block_count - i - 1) * sizeof *blocks);
    block_count--;

    // Merge adjacent free holes
    merge_holes();
}

static void C(void) {
    // Sort allocated blocks by start address
    if (block_count > 1) {
        qsort(blocks, block_count, sizeof *blocks, compare_blocks);
    }

    // Slide each block down to the next free address
    size_t cur = 0;
    for (size_t i = 0; i < block_count; i++) {
        blocks[i].start = cur;
        cur += blocks[i].size;
    }

    // Rebuild single free hole after all allocations
    hole_count = 1;
    holes[0].start = cur;
    holes[0].size  = total_mem - cur;
}

static void STAT(void) {
    // Print allocated blocks
    for (size_t i = 0; i < block_count; i++) {
        size_t s = blocks[i].start;
        size_t e = s + blocks[i].size - 1;
        printf("Addresses [%zu:%zu] Process %s\n", s, e, blocks[i].pid);
    }
    // Print free holes
    for (size_t i = 0; i < hole_count; i++) {
        size_t s = holes[i].start;
        size_t e = s + holes[i].size - 1;
        printf("Addresses [%zu:%zu] Free\n", s, e);
    }
}

// Parse a line of input into tokens and process the command
static void handle_command(char *line) {
    char *toks[8];
    int nt = 0;
    char *tok = strtok(line, " \t\n");
    while (tok && nt < 8) {
        toks[nt++] = tok;
        tok = strtok(NULL, " \t\n");
    }
    if (nt == 0) return;  // empty input

    // Process command token
    if (strcmp(toks[0], "RQ") == 0) {
        RQ(toks, nt);
    }
    else if (strcmp(toks[0], "RL") == 0) {
        RL(toks, nt);
    }
    else if (strcmp(toks[0], "C") == 0) {
        C();
    }
    else if (strcmp(toks[0], "STAT") == 0) {
        STAT();
    }
    else {
        printf("Error: Unknown command %s\n", toks[0]);
    }
}

// Set up memory allocation
void init_allocator(size_t max) {
    total_mem = max;
    hole_capacity = 4;
    holes = malloc(hole_capacity * sizeof *holes);
    if (!holes) {
        perror("malloc holes");
        exit(EXIT_FAILURE);
    }
    hole_count = 1;
    holes[0].start = 0;
    holes[0].size  = max;

    block_capacity = 4;
    blocks = malloc(block_capacity * sizeof *blocks);
    if (!blocks) {
        perror("malloc allocs");
        exit(EXIT_FAILURE);
    }
    block_count = 0;
}

// Read commands until 'X' to exit
void loop(void) {
    char line[128];
    while (1) {
        printf("allocator> ");
        if (!fgets(line, sizeof line, stdin))
            break;  // end of file
        if (line[0] == 'X' && (line[1] == '\n' || line[1] == '\0'))
            break;  // exit command
        handle_command(line);
    }
}