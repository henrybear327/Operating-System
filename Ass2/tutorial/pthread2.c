#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int myglobal;
void *thread_function(void *arg)
{
    int i, j;
    for (i = 0; i < 3; i++) {
        j = myglobal;
        j = j + 1;
        printf("myglobal in another thread = %d, j = %d\n", myglobal, j);
        fflush(stdout);
        sleep(1);
        myglobal = j;
    }
    return NULL;
}

int main(void)
{
    pthread_t mythread;
    int i;
    if (pthread_create(&mythread, NULL, thread_function, NULL)) {
        printf("error creating thread.");
        abort();
    }
    for (i = 0; i < 3; i++) {
        myglobal = myglobal + 1;
        printf("myglobal in main thread = %d\n", myglobal);
        fflush(stdout);
        sleep(5);
    }
    if (pthread_join(mythread, NULL)) { // pthread_join force the main process to wait for mythread to finish
        printf("error joining thread.");
        abort();
    }
    printf("\nmyglobal equals %d\n", myglobal);
    exit(0);
}