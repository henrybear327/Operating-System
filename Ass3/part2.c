#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

typedef long long ll;

int main()
{
    srand(time(NULL));

    ll size = 0;

    int *ptr[20];
    for(int i = 0; i < 20; i++) {
        ptr[i] = malloc(N * sizeof(int));
        if (ptr[i] == NULL) {
            break;
        }
        size++;

        for(int j = 0; j < N; j++)
            ptr[i][j] = rand() % N;
    }

    for(int i = 0; i < 20; i++) {
        free(ptr[i]);
    }

    return 0;
}
