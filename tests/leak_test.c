#include <stdio.h>
#include <stdlib.h>

// Minimal program that intentionally allocates and frees memory correctly.
// Useful as a small runtime target for Valgrind leak checking in CI.

int main(void) {
    const size_t N = 1000;
    int *arr = malloc(sizeof(int) * N);
    if (!arr) {
        fprintf(stderr, "malloc failed\n");
        return 2;
    }

    for (size_t i = 0; i < N; ++i) arr[i] = (int)i;

    // do a tiny sanity check
    if (arr[0] != 0 || arr[N-1] != (int)(N-1)) {
        fprintf(stderr, "sanity check failed\n");
        free(arr);
        return 3;
    }

    free(arr);
    printf("leak_test: OK\n");
    return 0;
}
