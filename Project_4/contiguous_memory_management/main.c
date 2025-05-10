#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <MAX-bytes>\n", argv[0]);
        return 1;
    }
    size_t MAX = strtoull(argv[1], NULL, 10);
    init_allocator(MAX);
    loop();
    return EXIT_SUCCESS;
}