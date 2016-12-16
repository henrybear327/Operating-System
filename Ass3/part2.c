#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N (int)1e8
#define M 20

typedef long long ll;

int main()
{
    srand(time(NULL));

    ll size = 0;

    int *ptr[M];
    for(int i = 0; i < M; i++) {
		printf("Now on %d\n", i);
        ptr[i] = malloc(N * sizeof(int));
        if (ptr[i] == NULL) {
            break;
        }
        size++;

        for(int j = 0; j < N; j++)
            ptr[i][j] = rand() % N;
    }

    for(int i = 0; i < M; i++) {
        free(ptr[i]);
    }

    return 0;
}
