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
1 - only messages, asserts, and merge sort correctness check, no array data
2 - messages, asserts, merge sort correctness check, and array data
*/
#define DEBUG 1

// define terminal colorful text output
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *originalData, *dataForSorting, *comparisionData;

// obtained by calling benchmarkOneThreadStdQsort() -- baseline comparator
int oneThreadStdQsortTime, totalDistanceCorrectAnswer;

int oneThreadMergeSortTime;

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
    free(comparisionData);
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
    totalDistanceCorrectAnswer =
        dataForSorting[data_size - 1] - dataForSorting[0];

    // get time taken
    oneThreadStdQsortTime = printTimeElapsed(start, "benchmarkOneThreadStdQsort");

    print_result(data_size, "benchmarkOneThreadStdQsort");

#if DEBUG != 0
    comparisionData = (int*) malloc(sizeof(int) * data_size);
    // copy the qsort answer to the comparisionData for the merge sort to compare
    memcpy(comparisionData, dataForSorting, sizeof(int) * data_size);
#endif
}

void mergeSortCombine(int left, int mid, int right)
{
    // merge two lists
    int pa = left, pb = mid, idx = 0;
    int *tmpData = (int*) malloc(sizeof(int) * (right - left));
    while(pa < mid && pb < right) {
        if(dataForSorting[pa] <= dataForSorting[pb])
            tmpData[idx++] = dataForSorting[pa++];
        else
            tmpData[idx++] = dataForSorting[pb++];
    }

    while(pa < mid)
        tmpData[idx++] = dataForSorting[pa++];
    while(pb < right)
        tmpData[idx++] = dataForSorting[pb++];

    // TODO: wait for mutex lock

    for(int i = 0; i < right - left; i++)
        dataForSorting[left + i] = tmpData[i];

    free(tmpData);
}

// [left, right)
void mergeSort(int left, int right)
{
    // printf("%d %d\n", left, right);
    if(right - left < 2)
        return;

    int mid = (left + right) / 2;
    mergeSort(left, mid);
    mergeSort(mid, right);
    // printf("%d %d %d\n", left, mid, right);
    mergeSortCombine(left, mid, right);
}

void benchmarkOneThreadMergeSort(int data_size)
{
    // prepare the array for sorting
    prepareArrayForSorting(data_size);

    // start merge sort!
    clock_t start = clock();

    mergeSort(0, data_size);

    // get time taken
    oneThreadMergeSortTime = printTimeElapsed(start, "benchmarkOneThreadMergeSort");

    print_result(data_size, "benchmarkOneThreadMergeSort");
#if DEBUG != 0
    // check merge sort solution against the qsort solution
    // ensure merge sort is correct
    for(int i = 0; i < data_size; i++)
        assert(dataForSorting[i] == comparisionData[i]);
#endif
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

    // Run the baseline sorting algorithm 1 -- qsort on 1 thread. (Record time)
    // Set correct comparing data (total diff)
    benchmarkOneThreadStdQsort(data_size);

    // Run the baseline sorting algorithm 2 -- merge sort on 1 thread. (Record time)
    benchmarkOneThreadMergeSort(data_size);

    cleanup();

    printTimeElapsed(start, "the entire program");

    return 0;
}
