#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024

typedef long long ll;

int main()
{
    ll size = 0;

    while (1) {
        int *data = malloc(N);
        if (data == NULL) {
            break;
        }
        size++;
    }

    printf("Allocated %lld blocks\n", size);

    return 0;
}
