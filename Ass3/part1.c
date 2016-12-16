#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N (int)1e9 // change to 40 * 1024 * 1024 = 41943040 for 40 MB

int main()
{
    srand(time(NULL));
    int n, *data = malloc(sizeof(int) * N);
    scanf("%d", &n);

    for (int i = 0; i < N; i++)
        data[i] = rand();


    int idx = rand() % N;
    printf("data[%d] = %d\n", idx, data[idx]);

    scanf("%d", &n);

    free(data);

    return 0;
}
