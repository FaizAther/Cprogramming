#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>

#include <errno.h>

#define N 10

int main (int argc, char **argv) {
    assert (argc >= 2);
    
    const char *pathname = argv[1];

    int fd = open(pathname,
                    O_RDWR);
                /*  O_RDONLY); */

    if (fd == -1) {
        perror ("unable to open file");
        return EXIT_FAILURE;
    }

    struct stat buf;
    int err = fstat(fd, &buf);
    
    if (fd == -1) {
        perror ("unable to open file");
        return EXIT_FAILURE;
    }

    int *ptr = (int *)mmap(NULL, buf.st_size,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE,
                /*  PROT_READ,
                    MAP_PRIVATE, */
                    fd, 0);

    assert(ptr != MAP_FAILED && N >= 2);
    close (fd);

    for (size_t i = 0; i < buf.st_size / 2; i++) {
        int j = buf.st_size - i - 1;
        int t = ptr[i];
        ptr[i] = ptr[j];
        ptr[j] = t;
    }

    ssize_t n = write (1, ptr, buf.st_size);
    assert (n == buf.st_size);

    err = munmap(ptr, buf.st_size);
    assert (err >= -1);

    return EXIT_SUCCESS;
}
