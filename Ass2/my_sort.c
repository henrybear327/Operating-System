#include <stdio.h>
#include <stdlib.h>

#define min(x,y) (x<y?x:y)

int main(int argc, char** argv)
{
	if(argc != 3) {
		fprintf(stderr, "Invalid arguments");
		return EXIT_FAILURE;
	}
	
	// my_sort rand_seed data_size
	int rand_seed = atoi(argv[1]);
	int data_size = atoi(argv[2]);

	// generate numbers
	srand(rand_seed);
	int data[data_size];
	for(int i = 0; i < data_size; i++)
		data[i] = rand();
	
	for(int i = 0; i < data_size; i++)
		printf("%d %d\n", i, data[i]);

	// create min(8, data_size) threads for sorting
	int threadToCreate = min(8, data_size);
	for(int i = 0; i < threadToCreate; i++) {
		// create threads
	}

	return 0;
}
