#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <time.h>

/*
0 - production mode
1 - only messages and asserts, no array data
2 - messages, asserts, and array data
*/
#define DEBUG 1

// define terminal colorful text output
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *originalData, *temporaryData, *dataForSorting;

// obtained by calling benchmarkOneThreadStdQsort() -- baseline comparator
int oneThreadStdQsortTime, totalDistanceCorrectAnswer;

struct sorting_parameter { // [l, r)
    int left_bound, right_bound;
};

inline int printTimeElapsed(int start, char *string)
{
    clock_t diff = clock() - start;
    int milliseconds = diff * 1000 / CLOCKS_PER_SEC;
    // user + system
    printf(CYAN "Total time taken by the %s is %d.%03d second(s)\n" NONE, string,
           milliseconds / 1000, milliseconds % 1000);

    return milliseconds;
}

void generateNumbersForSorting(int seed, int size)
{
    clock_t start = clock();

#if DEBUG != 0
    printf(GREEN "Generating %d numbers with seed %d for sorting...\n" NONE, size,
           seed);
#endif

    srand(seed);
    originalData = (int *)malloc(sizeof(int) * size);
    dataForSorting = (int *)malloc(sizeof(int) * size);
    temporaryData = (int *)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++)
        originalData[i] = rand();

    printf(GREEN "Generated %d numbers with seed %d for sorting...\n" NONE, size,
           seed);
    printTimeElapsed(start, "generator");
    printf("\n");

#if DEBUG == 2
    for (int i = 0; i < size; i++)
        printf("%d%c", originalData[i], i == size - 1 ? '\n' : ' ');
    printf("\n");
#endif
}

void print_result(int data_size, char *string)
{
    int distanceSum = 0;

#if DEBUG == 2
    printf(GREEN "The sorted array after doing %s is...\n" NONE, string);
    for (int i = 0; i < data_size; i++)
        printf("%d%c", dataForSorting[i], i == data_size - 1 ? '\n' : ' ');
#endif

    for (int i = 1; i < data_size; i++)
        distanceSum += dataForSorting[i] - dataForSorting[i - 1];
    printf(
        GREEN
        "The sum of distance between consecutive numbers after %s is %d\n" NONE,
        string, distanceSum);

    printf("\n");
#if DEBUG == 1
    assert(distanceSum == dataForSorting[data_size - 1] - dataForSorting[0]);
#endif
}

void cleanup()
{
    free(originalData);
    free(dataForSorting);
    free(temporaryData);
}

void prepareArrayForSorting(int data_size)
{
    memcpy(dataForSorting, originalData, sizeof(int) * data_size);
}

int cmp(const void *a, const void *b)
{
    return *((int *)a) - *((int *)b);
}

void benchmarkOneThreadStdQsort(int data_size)
{
    // prepare the array for sorting
    prepareArrayForSorting(data_size);

    // start qsort!
    clock_t start = clock();

    qsort(dataForSorting, data_size, sizeof(int), cmp);
    totalDistanceCorrectAnswer = dataForSorting[data_size - 1] - dataForSorting[0];

    oneThreadStdQsortTime = printTimeElapsed(start, "benchmarkOneThreadStdQsort");

    print_result(data_size, "benchmarkOneThreadStdQsort");
}

int main(int argc, char **argv)
{
#if DEBUG != 0
    printf(RED "********The debug mode is ON (mode %d)!********\n\n\n" NONE,
           DEBUG);
#endif
    clock_t start = clock();

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

    // Run the baseline sorting algorithm -- qsort on 1 thread.
    // Set correct comparing data
    benchmarkOneThreadStdQsort(data_size);

    cleanup();

    printTimeElapsed(start, "the entire program");

    return 0;
}
