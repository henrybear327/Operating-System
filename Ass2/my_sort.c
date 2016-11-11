#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define DEBUG 1 // debug mode: 1 is on, 0 is off

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

#define min(x, y) (x < y ? x : y)

int *dataForSorting, *resultAfterSorting;

void generateNumbersForSorting(int seed, int size)
{
    printf(GREEN "Generating %d numbers with seed %d for sorting...\n" NONE, size, seed);

    srand(seed);
    dataForSorting = (int*)malloc(sizeof(int) * size);
    resultAfterSorting = (int*)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++)
        dataForSorting[i] = rand();

    for (int i = 0; i < size; i++)
        printf("%d%c", dataForSorting[i], i == size - 1 ? '\n' : ' ');
    printf("\n");
}

void* sorting_function(void* arg)
{
    // arg is the start and end index for sorting

    // sort

    // write to resultAfterSorting

    // wait for mutex?

    return NULL;
}

void cleanup()
{
    free(dataForSorting);
}

int main(int argc, char **argv)
{
    if (argc != 3) { // check if all arguments are given
        //fprintf(stderr, "Insufficient arguments");
        printf(RED "Insufficient arguments! Program will terminate now\n" NONE);

        return EXIT_FAILURE;
    }

    // my_sort rand_seed data_size
    int rand_seed = atoi(argv[1]);
    int data_size = atoi(argv[2]);

    // generate numbers for sorting
    generateNumbersForSorting(rand_seed, data_size);

    // create min(8, data_size) threads for sorting
    int threadsToCreate = min(8, data_size);
    printf(GREEN "Creating %d threads and start sorting...\n" NONE, threadsToCreate);

    pthread_t mythread[threadsToCreate];
    struct bound arguments[threadsToCreate];
    for (int i = 0; i < threadsToCreate; i++) {
        // set bound

        // create threads, passing in the bound
        if (pthread_create(&mythread[i], NULL, sorting_function, &arguments[i])) {
            perror("Error creating threads");
            printf(RED "Error creating thread. Errno = %d" NONE, errno);
            abort();
        }
    }

    // merge two consecutive bounds

    printf(GREEN "Sorting is done!\n" NONE, threadsToCreate);
    // print the result
    long long int distanceSum = 0;
    for(int i = 1; i < data_size; i++)
        distanceSum += resultAfterSorting[i] - resultAfterSorting[i - 1];
    printf(GREEN "The sum of distance between consecutive numbers is %lld\n" NONE, distanceSum);

    cleanup();

    return 0;
}
