#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N (40 * 1024 * 1024)

int main()
{
    srand(time(NULL));
    int n, *data = malloc(N);
    scanf("%d", &n);

    for (int i = 0; i < N / 4; i++)
        data[i] = rand();

    int idx = rand() % (N / 4);
    printf("data[%d] = %d\n", idx, data[idx]);

    scanf("%d", &n);

    free(data);

    return 0;
}
