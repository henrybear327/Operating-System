#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"
#define CYAN "\033[0;36m"

int getShareMemory(key_t key)
{
	int shm_id;
	shm_id=shmget(key,4096,IPC_CREAT|0666);   
	if(shm_id==-1)
	{
		perror("Get shared memory error");
		printf(RED "Can't get share memory" NONE);
		return -1;
	}

	return shm_id; 
}

void deleteSharedMemory(int id, struct shmid_ds* shmbuffer)
{
	int error = shmctl(id, IPC_RMID, shmbuffer);	
	if(error == 0)
		printf(GREEN "Shared memory freed successfully!\n" NONE); // errno == 2??? Why?
	else {
		perror("Free shared memory error");
		printf(RED "Free error: errno = %d\n" NONE, errno);
	}
}

int main()
{
	// ipcs -m

	printf(CYAN "Please enter a segment_id, or -1 to create a new shared memory: " NONE);
	int segment_id;
	scanf("%d", &segment_id);

	if(segment_id == -1) {
		// get new shared memory
		int user_input_key;
		printf(CYAN "Please enter a key for creating the shared memory: " NONE);
		scanf("%d", &user_input_key);
		segment_id = getShareMemory(user_input_key);
		if(segment_id == -1)
			return 0;
	}

	// get shared memory struct
	struct shmid_ds shmbuffer;
	int error = shmctl(segment_id, IPC_STAT, &shmbuffer); 
	if(error == 0) {
		printf(GREEN "Just created a new shared memory\nThe fields are...\n" NONE);

		// print shared memory info
		printf("Segment ID: %d\n",segment_id);
		printf("Key: %d\n",(int)shmbuffer.shm_perm.key);
		printf("Mode: %u\n",shmbuffer.shm_perm.mode);
		printf("Owner uid: %u\n",shmbuffer.shm_perm.uid);
		printf("Size: %lu\n",shmbuffer.shm_segsz);
		printf("Number of attaches: %d\n",shmbuffer.shm_nattch);

		// free memory 
		deleteSharedMemory(segment_id, &shmbuffer);
	} else {
		perror("Get shared memory information error");
		printf(RED "errno = %d\n" NONE, errno);
		if(errno == 13)
			printf(RED "permission denied\n" NONE);
		if(errno == 22)
			printf(RED "Invalid argument\n" NONE);
	}

	return 0;
}
