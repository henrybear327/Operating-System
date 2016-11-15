#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <semaphore.h>
#include <time.h>

/*
0 - Production mode
1 - Only messages, asserts, and sorting result correctness checks are performed.
2 - Messages, asserts, sorting result correctness checks, and print out all array data
*/
#define DEBUG 1

// Maximun cores available for use, usually 16 is enough
#define MAX_PROCESSER 16

// define terminal colorful text output
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *originalData, *dataForSorting, *comparisionData;

// obtained by calling benchmarkOneThreadStdQsort()
int oneThreadStdQsortTime, totalDistanceCorrectAnswer;

// obtained by calling benchmarkOneThreadMergeSort()
int oneThreadMergeSortTime;

int printTimeElapsed(int start, char *string)
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
    comparisionData = (int *)malloc(sizeof(int) * data_size);
    // copy the qsort answer to the comparisionData for the merge sort to compare
    memcpy(comparisionData, dataForSorting, sizeof(int) * data_size);
#endif
}

void mergeSortCombine(int left, int mid, int right)
{
    // printf("%d %d %d %d\n", left, mid, right, right - left);

    // merge two lists
    int pa = left, pb = mid;
    int *tmpData = (int*) malloc(sizeof(int) * (right - left));
    if(tmpData == NULL) {
        printf(RED "Malloc failed! (left %d right %d)\n", left, right);
        perror("Malloc failed");
        abort();
    }

    int idx = 0;
    while (pa < mid && pb < right) {
        if (dataForSorting[pa] <= dataForSorting[pb])
            tmpData[idx++] = dataForSorting[pa++];
        else
            tmpData[idx++] = dataForSorting[pb++];
    }

    while (pa < mid)
        tmpData[idx++] = dataForSorting[pa++];
    while (pb < right)
        tmpData[idx++] = dataForSorting[pb++];

    for (int i = 0; i < right - left; i++)
        dataForSorting[left + i] = tmpData[i];

    free(tmpData);
}

// [left, right)
void mergeSort(int left, int right)
{
    // printf("%d %d\n", left, right);
    if (right - left < 2)
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
    oneThreadMergeSortTime =
        printTimeElapsed(start, "benchmarkOneThreadMergeSort");

#if DEBUG != 0
    // check merge sort solution against the qsort solution
    // ensure merge sort is correct
    for (int i = 0; i < data_size; i++)
        assert(dataForSorting[i] == comparisionData[i]);
    printf(GREEN "The sorted result is verified - correct!\n" NONE);
#endif

    print_result(data_size, "benchmarkOneThreadMergeSort");
}

/********************************************************/
/* Multi-threaded stuff starts here!                    */
/********************************************************/

pthread_mutex_t mutex;

struct sorting_parameter { // [l, r)
    int left_bound, right_bound;
    int* dataToSort;
};

int myThreadIndex;
pthread_t mythread[MAX_PROCESSER];
struct sorting_parameter param[MAX_PROCESSER];

int done;

void *multiThreadMergeSort(void *argument)
{
    struct sorting_parameter *param = (struct sorting_parameter *)argument;
    int left = param->left_bound;
    int right = param->right_bound;

    clock_t start = clock();

    // call the normal mergeSort to do the job!
    // mergeSort(left, right);
    qsort(param->dataToSort, right - left, sizeof(int), cmp);
    memcpy(dataForSorting + left, param->dataToSort, sizeof(int) * (right - left));


    int diff = clock() - start;
    int milliseconds = diff * 1000 / CLOCKS_PER_SEC;
    // user + system
    printf(CYAN "Total time taken by the %s is %d.%03d second(s)\n" NONE,
           "Done sub-sorting", milliseconds / 1000, milliseconds % 1000);

    start = clock();

    pthread_mutex_lock(&mutex);
    done++;
    pthread_mutex_unlock(&mutex);

    diff = clock() - start;
    milliseconds = diff * 1000 / CLOCKS_PER_SEC;
    // user + system
    printf(CYAN "Total time taken by the %s is %d.%03d second(s)\n" NONE,
           "Wait for Mutex", milliseconds / 1000, milliseconds % 1000);

    pthread_exit(NULL);
}

void createSortingThread(int left, int right, int threshold)
{
    // printf("%d %d\n", left, right);
    pthread_mutex_lock(&mutex);

    param[myThreadIndex].left_bound = left;
    param[myThreadIndex].right_bound = right;
    param[myThreadIndex].dataToSort = (int*) malloc(sizeof(int) * (right - left));
    memcpy(param[myThreadIndex].dataToSort, dataForSorting + left, sizeof(int) * (right - left));

    if (pthread_create(&mythread[myThreadIndex], NULL, multiThreadMergeSort,
                       &param[myThreadIndex]) != 0) {
        printf(RED "Error creating thread under threshold %d. (left = %d, right "
               "= %d)." NONE,
               threshold, left, right);
        perror("Error creating thread");

        abort();
    }

    myThreadIndex++;

    pthread_mutex_unlock(&mutex);
}

// [left, right)
void multiThreadMergeSortCreater(int left, int right, int threshold)
{
    // printf("%d %d\n", left, right);
    if (right - left < 2)
        return;

    int mid = (left + right) / 2;

    //  recursive call until threshold is met
    if (right - left <= threshold * 2) {
        // create left
        createSortingThread(left, mid, threshold);

        // create right
        createSortingThread(mid, right, threshold);

        return;
    } else {
        multiThreadMergeSortCreater(left, mid, threshold);
        multiThreadMergeSortCreater(mid, right, threshold);
    }
    // printf("%d %d %d\n", left, mid, right);
}

void multiThreadMergeSortMerger(int left, int right, int threshold)
{
    // printf("%d %d\n", left, right);
    if (right - left < 2)
        return;

    int mid = (left + right) / 2;

    //  recursive call until threshold is met
    if (right - left <= threshold * 2) {
        // printf("%d %d %d\n", left, mid, right);
        mergeSortCombine(left, mid, right);
    } else {
        multiThreadMergeSortMerger(left, mid, threshold);
        multiThreadMergeSortMerger(mid, right, threshold);
        mergeSortCombine(left, mid, right);
    }
    // printf("%d %d %d\n", left, mid, right);

    return;
}

void benchmarkMultiThreadMergeSort(int data_size, int threshold)
{
    assert(threshold > 0);

    // prepare the array for sorting
    prepareArrayForSorting(data_size);

    // Prepare mutex
    pthread_mutex_init(&mutex, NULL);

    // Prepare thread index
    myThreadIndex = 0;
    done = 0;

    // start merge sort!
    clock_t start = clock();

    multiThreadMergeSortCreater(0, data_size, threshold);

    clock_t start1;
    while (1) {
        pthread_mutex_lock(&mutex);
        if (done == myThreadIndex) {
            printTimeElapsed(start, "done sorting");
            start1 = clock();

            multiThreadMergeSortMerger(0, data_size, threshold);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }

    printTimeElapsed(start1, "done merging");

    // get time taken
    int multiThreadMergeSortTime =
        printTimeElapsed(start, "benchmarkMultiThreadMergeSort");

    print_result(data_size, "benchmarkMultiThreadMergeSort");

#if DEBUG != 0
    // check multi-thread merge sort solution against the qsort solution
    // ensure merge sort is correct
    for (int i = 0; i < data_size; i++)
        assert(dataForSorting[i] == comparisionData[i]);
    printf(GREEN "The sorted result is verified - correct!\n" NONE);
#endif

    // get improvement data
    double improvementOverQsort =
        oneThreadStdQsortTime == 0
        ? 0
        : (double)(oneThreadStdQsortTime - multiThreadMergeSortTime) /
        oneThreadStdQsortTime;
    double improvementOverMergeSort =
        oneThreadMergeSortTime == 0
        ? oneThreadMergeSortTime
        : (double)(oneThreadMergeSortTime - multiThreadMergeSortTime) /
        oneThreadMergeSortTime;

    printf(CYAN
           "Multi-threaded merge sort (threshold %d) is:\n"
           "%.02f%% %s than single-threaded qsort (%d.%03d vs %d.%03d), and \n"
           "%.02f%% %s than single-threaded merge sort (%d.%03d vs "
           "%d.%03d).\n\n" NONE,
           threshold, improvementOverQsort * 100,
           improvementOverQsort >= 0 ? "faster" : "slower",
           multiThreadMergeSortTime / 1000, multiThreadMergeSortTime % 1000,
           oneThreadStdQsortTime / 1000, oneThreadStdQsortTime % 1000,
           improvementOverMergeSort * 100,
           improvementOverMergeSort >= 0 ? "faster" : "slower",
           multiThreadMergeSortTime / 1000, multiThreadMergeSortTime % 1000,
           oneThreadMergeSortTime / 1000, oneThreadMergeSortTime % 1000);
}

int main(int argc, char **argv)
{
#if DEBUG != 0
    printf(RED "********The debug mode is ON (mode %d)!********\n" NONE, DEBUG);
    printf(RED "This is a %ld-core CPU\n\n\n" NONE,
           sysconf((_SC_NPROCESSORS_ONLN)));
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

    // Run the baseline sorting algorithm 1
    // qsort on 1 thread. (Record time)
    // Set correct comparing data (total diff)
    benchmarkOneThreadStdQsort(data_size);

    // Run the baseline sorting algorithm 2
    // merge sort on 1 thread. (Record time)
    benchmarkOneThreadMergeSort(data_size);

    int threshold = ceil((double)data_size / (sysconf(_SC_NPROCESSORS_ONLN)));
    // benchmarkMultiThreadMergeSort(data_size, threshold);

    cleanup();

    printTimeElapsed(start, "entire program");

    return 0;
}
