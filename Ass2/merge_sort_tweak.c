#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NOTHREADS 2

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *a;

void generateNumbersForSorting(int seed, int size)
{
    printf(GREEN "Generating %d numbers with seed %d for sorting...\n" NONE, size,
           seed);

    srand(seed);
    a = (int *)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++)
        a[i] = rand();

    for (int i = 0; i < size; i++)
        printf("%d%c", a[i], i == size - 1 ? '\n' : ' ');
    printf("\n");
}

typedef struct node {
    int i;
    int j;
} NODE;

void merge(int i, int j)
{
    int mid = (i + j) / 2;
    int ai = i;
    int bi = mid + 1;

    int newa[j - i + 1], newai = 0;

    while (ai <= mid && bi <= j) {
        if (a[ai] > a[bi])
            newa[newai++] = a[bi++];
        else
            newa[newai++] = a[ai++];
    }

    while (ai <= mid) {
        newa[newai++] = a[ai++];
    }

    while (bi <= j) {
        newa[newai++] = a[bi++];
    }

    for (ai = 0; ai < (j - i + 1); ai++)
        a[i + ai] = newa[ai];
}

void *merge_sort(void *a)
{
    NODE *p = (NODE *)a;
    NODE n1, n2;
    int mid = (p->i + p->j) / 2;
    pthread_t tid1, tid2;
    int ret;

    n1.i = p->i;
    n1.j = mid;

    n2.i = mid + 1;
    n2.j = p->j;

    if (p->i >= p->j)
        return NULL;

    ret = pthread_create(&tid1, NULL, merge_sort, &n1);
    if (ret) {
        printf("%d %s - unable to create thread - ret - %d\n", __LINE__,
               __FUNCTION__, ret);
        exit(1);
    }

    ret = pthread_create(&tid2, NULL, merge_sort, &n2);
    if (ret) {
        printf("%d %s - unable to create thread - ret - %d\n", __LINE__,
               __FUNCTION__, ret);
        exit(1);
    }

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    merge(p->i, p->j);
    pthread_exit(NULL);
}

void print_result(int data_size)
{
    printf(GREEN "Sorting is done!\n" NONE);
    int distanceSum = 0;

    for (int i = 0; i < data_size; i++)
        printf("%d%c", a[i], i == data_size - 1 ? '\n' : ' ');

    for (int i = 1; i < data_size; i++)
        distanceSum += a[i] - a[i - 1];
    printf(GREEN
           "The sum of distance between consecutive numbers is %d = %d\n" NONE,
           distanceSum, a[data_size - 1] - a[0]);
}

int main(int argc, char **argv)
{
    if (argc != 3) { // check if all arguments are given
        // fprintf(stderr, "Insufficient arguments");
        printf(RED "Insufficient arguments! Program will terminate now\n" NONE);

        return EXIT_FAILURE;
    }

    // my_sort rand_seed data_size
    int rand_seed = atoi(argv[1]);
    int data_size = atoi(argv[2]);

    // generate numbers for sorting
    generateNumbersForSorting(rand_seed, data_size);

    NODE m;
    m.i = 0;
    m.j = data_size - 1;
    pthread_t tid;

    int ret;

    ret = pthread_create(&tid, NULL, merge_sort, &m);
    if (ret) {
        printf("%d %s - unable to create thread - ret - %d\n", __LINE__,
               __FUNCTION__, ret);
        exit(1);
    }

    pthread_join(tid, NULL);

    print_result(data_size);

    // pthread_exit(NULL);
    return 0;
}
