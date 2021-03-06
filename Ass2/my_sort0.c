#include <assert.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/*
0 - Production mode
1 - Only messages, asserts, and sorting result correctness checks are performed.
2 - Messages, asserts, sorting result correctness checks, and print out all
array data
*/
#define DEBUG 1

#define MAX_PROCESSER 16

// define terminal colorful text output
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int *originalData, *dataForSorting, *comparisionData, *mergeSortTmpArray;

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
    assert(left < mid && mid < right);

    // merge two lists
    int pa = left, pb = mid;

    int idx = left;
    while (pa < mid && pb < right) {
        if (dataForSorting[pa] <= dataForSorting[pb])
            mergeSortTmpArray[idx++] = dataForSorting[pa++];
        else
            mergeSortTmpArray[idx++] = dataForSorting[pb++];
    }

    while (pa < mid)
        mergeSortTmpArray[idx++] = dataForSorting[pa++];
    while (pb < right)
        mergeSortTmpArray[idx++] = dataForSorting[pb++];

    for (int i = 0; i < right - left; i++)
        dataForSorting[left + i] = mergeSortTmpArray[left + i];
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
    mergeSortTmpArray = (int *)malloc(sizeof(int) * data_size);

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

    free(mergeSortTmpArray);
}

/********************************************************/
/* Multi-threaded stuff starts here!                    */
/********************************************************/

pthread_mutex_t mutex;

struct sorting_parameter { // [l, r)
    int left_bound, right_bound;
    int segmentSize;
    int *dataToSort;
};

int myThreadIndex, done;
pthread_t mythread[MAX_PROCESSER];
struct sorting_parameter param[MAX_PROCESSER];

void multiThreadMerger(int left, int right) // [left, right)
{
    // printf("%d %d\n", left, right);
    if (right - left <= 1)
        return;

    int mid = (left + right) / 2;
    multiThreadMerger(left, mid);
    multiThreadMerger(mid, right);

    // printf("%d %d %d, %d %d %d\n", left, mid, right - 1,
    // param[left].left_bound, param[mid].left_bound, param[right -
    // 1].right_bound);
    mergeSortCombine(param[left].left_bound, param[mid].left_bound,
                     param[right - 1].right_bound);
}

void *multiThreadSorting(void *argument)
{
    struct sorting_parameter *param = (struct sorting_parameter *)argument;
    int segmentSize = param->segmentSize;
    int *dataToSort = param->dataToSort;

    clock_t start = clock();

    // printf("%p %d %d\n", dataToSort, segmentSize, sizeof(int));
    qsort(dataToSort, segmentSize, sizeof(int), cmp);

    printTimeElapsed(start, "qsort in thread");

    pthread_exit(NULL);
}

void createSortingThread(struct sorting_parameter *arg)
{
    pthread_mutex_lock(&mutex);

    // printf("Create threads: %d %d\n", arg->left_bound, arg->right_bound);

    if (pthread_create(&mythread[myThreadIndex], NULL, multiThreadSorting, arg) !=
        0) {
        printf(RED "Error creating thread (left = %d, right = %d).\n" NONE,
               arg->left_bound, arg->right_bound);
        perror("Error creating thread");

        abort();
    }

    myThreadIndex++;

    pthread_mutex_unlock(&mutex);
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

    clock_t start = clock();

    // assign struct for sorting
    int segmentSize = data_size / threshold;

    // [left, right)
    param[0].left_bound = 0;
    param[0].right_bound = segmentSize;
    param[0].right_bound =
        param[0].right_bound > data_size ? data_size : param[0].right_bound;
    param[0].segmentSize = param[0].right_bound - param[0].left_bound;
    param[0].dataToSort = dataForSorting + param[0].left_bound;
    for (int i = 1; i < threshold; i++) {
        param[i].left_bound = param[i - 1].right_bound;
        param[i].right_bound = param[i - 1].right_bound + segmentSize;
        if (threshold - 1 == i)
            param[i].right_bound = data_size;
        param[i].segmentSize = param[i].right_bound - param[i].left_bound;
        param[i].dataToSort = dataForSorting + param[i].left_bound;
    }

    // create threads for sorting
    for (int i = 0; i < threshold; i++) {
        createSortingThread(&param[i]);
        // printf("%d: %d %d %d\n", i, param[i].left_bound, param[i].right_bound,
        // param[i].segmentSize);
    }

    // wait for sorting to be done
    for (int i = 0; i < threshold; i++) {
        if (pthread_join(mythread[i], NULL)) { // pthread_join force the main
            // process to wait for mythread to
            // finish
            printf(RED "Error joining thread %d\n", i);
            perror("Joining threads failed");
            abort();
        }
    }

    printTimeElapsed(start, "multithread sorting");

    // merge
    clock_t mergeStart = clock();
    printf("threshold %d\n", threshold);
    multiThreadMerger(0, threshold);
    printTimeElapsed(mergeStart, "multithread merging");

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

    int threshold = sysconf(_SC_NPROCESSORS_ONLN);
    struct timeval ss;
    gettimeofday(&ss, NULL);

    benchmarkMultiThreadMergeSort(data_size, threshold);

    struct timeval ee;
    gettimeofday(&ee, NULL);
    unsigned long diff =
        1000000 * (ee.tv_sec - ss.tv_sec) + ee.tv_usec - ss.tv_usec;
    printf(CYAN "Muitithread sorting takes %ld.%03ld\ns" NONE, diff / 1000000,
           diff / 1000 % 1000);

    // cleanup();

    printTimeElapsed(start, "entire program");

    return 0;
}
