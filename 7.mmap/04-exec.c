#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
    assert (argc >= 2);
    const char *path = argv[1];

    int fd = open (path, O_RDWR);
    assert (fd >= 0);

    struct stat buf;
    int err = fstat (fd, &buf);
    assert (err >= 0);

    void *ptr = mmap(NULL, buf.st_size,
                    PROT_EXEC,
                    MAP_PRIVATE,
                    fd, 0);
    assert (ptr != MAP_FAILED);
    close (fd);

    ((void (*)(void))ptr)();


    return 0;
}
