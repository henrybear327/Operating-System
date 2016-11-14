#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define DEBUG 1

// define terminal colorful text output
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *dataForSorting, *temporaryData;

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

    temporaryData = (int *)malloc(sizeof(int) * size);
}

int cmp(const void *a, const void *b)
{
    return *((int *)a) - *((int *)b);
}

void print_result(int data_size)
{
    printf(GREEN "Sorting is done!\n" NONE);
    int distanceSum = 0;

    for (int i = 0; i < data_size; i++)
        printf("%d%c", dataForSorting[i], i == data_size - 1 ? '\n' : ' ');
    for (int i = 1; i < data_size; i++)
        distanceSum += dataForSorting[i] - dataForSorting[i - 1];
    printf(GREEN
           "The sum of distance between consecutive numbers is %d\n" NONE,
           distanceSum);

#if DEBUG == 1
    assert(distanceSum == dataForSorting[data_size - 1] - dataForSorting[0]);
#endif
}

void cleanup()
{
    free(dataForSorting);
    free(temporaryData);
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

    // print the result
    print_result(data_size);

    cleanup();

    return 0;
}
