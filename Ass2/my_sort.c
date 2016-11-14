#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <pthread.h>
#include <unistd.h>

int mode = 1; // mode 0: multithread, mode 1: simple qsort

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

#define min(x, y) (x < y ? x : y)

int *dataForSorting, *resultAfterSorting;

struct sorting_parameter { // [l, r)
    int left_bound, right_bound;
};

void generateNumbersForSorting(int seed, int size)
{
    printf(GREEN "Generating %d numbers with seed %d for sorting...\n" NONE, size,
           seed);

    srand(seed);
    dataForSorting = (int *)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++)
        dataForSorting[i] = rand();

    for (int i = 0; i < size; i++)
        printf("%d%c", dataForSorting[i], i == size - 1 ? '\n' : ' ');
    printf("\n");

    resultAfterSorting = (int *)malloc(sizeof(int) * size);
}

int cmp(const void *a, const void *b)
{
    return *((int *)a) - *((int *)b);
}

// sorting in non-decreasing order
void *sorting_function(void *arg)
{
    struct sorting_parameter *param = (struct sorting_parameter *)arg;

    int elementCount = param->right_bound - param->left_bound;
    qsort(dataForSorting + param->left_bound, elementCount, sizeof(int), cmp);

    // for(int i = param->left_bound; i < param->right_bound; i++)
    //     printf("lp = %d %d\n", param->left_bound, dataForSorting[i]);

    // wait for mutex??

    return NULL;
}

void merging_function(struct sorting_parameter *param)
{
    int size[2];
    for (int i = 0; i < 2; i++)
        size[i] = param[i].right_bound - param[i].left_bound;

    int pa = param[0].left_bound, pb = param[1].left_bound, idx = 0;
    for (idx = 0;
         pa - param[0].left_bound < size[0] && pb - param[1].left_bound < size[1];
         idx++) {
        if (dataForSorting[pa] <= dataForSorting[pb])
            resultAfterSorting[idx] = dataForSorting[pa++];
        else
            resultAfterSorting[idx] = dataForSorting[pb++];
    }
    // printf("%d %d\n", pa, pb);

    while (pa - param[0].left_bound < size[0])
        resultAfterSorting[idx++] = dataForSorting[pa++];
    while (pb - param[1].left_bound < size[1])
        resultAfterSorting[idx++] = dataForSorting[pb++];
}

void print_result(int data_size)
{
    printf(GREEN "Sorting is done!\n" NONE);
    int distanceSum = 0;
    if (mode == 0) {
        for (int i = 0; i < data_size; i++)
            printf("%d%c", resultAfterSorting[i], i == data_size - 1 ? '\n' : ' ');
        for (int i = 1; i < data_size; i++)
            distanceSum += resultAfterSorting[i] - resultAfterSorting[i - 1];
        printf(
            GREEN "The sum of distance between consecutive numbers is %d = %d\n" NONE,
            distanceSum, resultAfterSorting[data_size - 1] - resultAfterSorting[0]);
    } else {
        for (int i = 0; i < data_size; i++)
            printf("%d%c", dataForSorting[i], i == data_size - 1 ? '\n' : ' ');
        for (int i = 1; i < data_size; i++)
            distanceSum += dataForSorting[i] - dataForSorting[i - 1];
        printf(
            GREEN "The sum of distance between consecutive numbers is %d = %d\n" NONE,
            distanceSum, dataForSorting[data_size - 1] - dataForSorting[0]);
    }
}

void cleanup()
{
    free(dataForSorting);
    free(resultAfterSorting);
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

    if (mode == 1) {
        qsort(dataForSorting, data_size, sizeof(int), cmp);
        print_result(data_size);

        return 0;
    }

    // special case, no need to sort
    if (data_size == 1) {
        print_result(data_size);

        return 0;
    }

    // Cut the array into 2 parts
    pthread_t mythread[2];
    struct sorting_parameter *param =
        (struct sorting_parameter *)malloc(sizeof(struct sorting_parameter) * 2);

    // set sorting parameter
    param[0].left_bound = 0;
    param[0].right_bound = data_size / 2 + 1;
    param[1].left_bound = data_size / 2 + 1;
    param[1].right_bound = data_size;

    // perform sorting on each of the them
    for (int i = 0; i < 2; i++) {
        // create thread
        if (pthread_create(&mythread[i], NULL, sorting_function, &param[i]) != 0) {
            printf(RED "Error creating thread." NONE);
            perror("Error creating thread");

            return EXIT_FAILURE;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(mythread[i], NULL);
    }

    // merge the result
    merging_function(param);

    // print the result
    print_result(data_size);

    cleanup();

    return 0;
}
