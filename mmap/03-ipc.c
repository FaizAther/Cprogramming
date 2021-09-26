#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <assert.h>

#define N 10

//int xs[N];

int *xs;

int main(int argc, char *argv[]) {
   
    xs = (int *)mmap(NULL, sizeof(int) * N,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS,
                    0, 0);
    
    assert(xs != NULL);

    for (int i = 0; i < N; i++) {
        xs[i] = i + 1;
    }

    pid_t cp = fork();

    if (cp == 0) {  // child
        
        printf("in child\n");
        
        for (int i = 0; i < N / 2; i++) {
            int t = xs[i];
            xs[i] = xs[N - i - 1];
            xs[N - i - 1] = t;
        }

        printf("Child: %d", xs[0]);
        for (int i = 1; i < N; i++) {
            printf(" %d", xs[i]);
        }
        printf("\n");
    } else {        // parent
        printf("in parent, wait for child\n");
        int status;
        pid_t w = waitpid(cp, &status, 0);
        printf("Parent: %d", xs[0]);
        for (int i = 1; i < N; i++) {
            printf(" %d", xs[i]);
        }
        printf("\n");
    }
    int err = munmap(xs, sizeof(int) * N);    
    assert(err != -1);
    return 0;
}
