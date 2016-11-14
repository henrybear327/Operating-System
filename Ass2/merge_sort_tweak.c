#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOTHREADS 2

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *a;
int *newa;

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
    int left_bound;
    int right_bound;
} NODE;

void merge(int i, int j)
{
    int mid = (i + j) / 2;
    int ai = i;
    int bi = mid + 1;

    int newai = 0;

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


    for (ai = 0; ai < (j - i + 1); ai++) {
        // printf("%d = %d\n", i + ai, newa[ai]);
        a[i + ai] = newa[ai];
    }

    // memcpy(a + i, newa, sizeof(int) * (j - i + 1));
}

void *merge_sort(void *a)
{
    NODE *p = (NODE *)a;
    if (p->left_bound >= p->right_bound)
        return NULL;

    NODE n1, n2;
    int mid = (p->left_bound + p->right_bound) / 2;
    // pthread_t tid1, tid2;

    n1.left_bound = p->left_bound;
    n1.right_bound = mid;

    n2.left_bound = mid + 1;
    n2.right_bound = p->right_bound;

    // printf("%d %d %d\n", p->left_bound, mid, p->right_bound);

    merge_sort(&n1);
    merge_sort(&n2);
    /*
    if (pthread_create(&tid1, NULL, merge_sort, &n1)) {
        perror("Error creating left thread\n");
        abort();
    }

    if (pthread_create(&tid2, NULL, merge_sort, &n2)) {
        perror("Error creating right thread\n");
        abort();
    }

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    */

    merge(p->left_bound, p->right_bound);
    // pthread_exit(NULL);
    return NULL;
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

    newa = (int*) malloc(sizeof(int) * data_size);

    // generate numbers for sorting
    generateNumbersForSorting(rand_seed, data_size);

    NODE ml, mr;
    ml.left_bound = 0;
    ml.right_bound = data_size / 2;

    mr.left_bound = data_size / 2 + 1;
    mr.right_bound = data_size - 1;
    pthread_t tidl, tidr;

    if (pthread_create(&tidl, NULL, merge_sort, &ml)) {
        perror("Error creating left thread\n");
        abort();
    }

    if(pthread_create(&tidr, NULL, merge_sort, &mr)) {
        perror("Error creating left thread\n");
        abort();
    }

    pthread_join(tidl, NULL);
    pthread_join(tidr, NULL);

    merge(0, data_size - 1);

    print_result(data_size);

    // pthread_exit(NULL);
    return 0;
}
