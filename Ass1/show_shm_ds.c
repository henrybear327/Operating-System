#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define GREEN "\033[0;32;32m"

int getShareMemory()
{
	int shm_id;
	key_t key = 0;
	shm_id=shmget(key,4096,IPC_CREAT|0666);   
	if(shm_id==-1)
	{
		printf("shmget error");
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
		printf("errno = %d\n", errno);
	}
}

int main()
{
	// ipcs -m
	int segment_id = getShareMemory();
	printf("segment_id = %d\n", segment_id);
	//int segment_id = 1769490;
	//if(segment_id == -1)
		//return 0;
	
	// get shared memory struct
	struct shmid_ds shmbuffer;
	int error = shmctl(segment_id, IPC_STAT, &shmbuffer); 
	if(error == 0) {
		printf(GREEN "Shared memory info\n" NONE);
	
		// print shared memory info
		printf("Segment ID: %d\n",segment_id);
		printf("Key: %d\n",(int)shmbuffer.shm_perm.key);
		printf("Mode: %u\n",shmbuffer.shm_perm.mode);
		printf("Owner uid: %u\n",shmbuffer.shm_perm.uid);
		printf("Size: %lu\n",shmbuffer.shm_segsz);
		printf("Number of attatch: %d\n",shmbuffer.shm_nattch);

		// get address
		char *shmaddr = (char *)shmat(segment_id, NULL, 0);
		if ( (int)shmaddr == -1 ) {
			printf("get shmaddr failed\n");
			return 0;
		}

		// free memory 
		deleteSharedMemory(segment_id, &shmbuffer);
		
		/*
		// detach all people
		error = shmdt(shmaddr);
		if(error == 0) {
			printf("Shared memory %d freed\n", segment_id);
			printf("errno = %d\n", errno);
		} else {
			printf("Shared memory can't be freed, errno = %d\n", errno);
		}
		*/
	} else {
		printf("errno = %d\n", errno);
		if(errno == 13)
			printf("permission denied\n");
	}

	return 0;
}
