#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>

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

int main()
{
	// ipcs -m
	// int segment_id = getShareMemory();
	// printf("segment_id = %d\n", segment_id);
	int segment_id = 65555;
	if(segment_id == -1)
		return 0;

	struct shmid_ds shmbuffer;

	int error = shmctl(segment_id, IPC_STAT, &shmbuffer);

	if(error == 0) {
		printf("ok\n");

		printf("Segment ID: %d\n",segment_id);
		printf("Key: %d\n",(int)shmbuffer.shm_perm.key);
		printf("Mode: %u\n",shmbuffer.shm_perm.mode);
		printf("Owner uid: %u\n",shmbuffer.shm_perm.uid);
		printf("Size: %lu\n",shmbuffer.shm_segsz);
		printf("Number of attatch: %d\n",shmbuffer.shm_nattch);
	} else {
		printf("errno = %d\n", errno);
		if(errno == 13)
			printf("permission denied\n");
	}

	return 0;
}
