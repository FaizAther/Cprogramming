#include <stdio.h>
#include <assert.h>

#include <sys/mman.h>

#define N 10

int main (void) {

    int *ptr = (int *)mmap(NULL, N * sizeof(int),
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    0, 0);

    assert(ptr != MAP_FAILED && N >= 2);

    ptr[0] = 0; ptr[1] = 1;

    for (int i = 2; i < N; i++) {
        ptr[i] = ptr[i - 1] + ptr[i - 2];
    }

    int err = munmap(ptr, N * sizeof(int));
    assert (err >= -1);

    return 0;
}
