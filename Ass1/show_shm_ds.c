#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>

int main()
{
	int segment_id;
	shm_ds shmbuffer;

	shmctl(segmsent_id, IPC_STAT, &shmbuffer);

	return 0;
}
