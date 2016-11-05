#include <unistd.h>
#include <stdio.h>

int main(void)
{
	printf("The very first process id = %d\n", getpid());
	printf("The very first process\'s parent ID: %d\n", getppid());
	fork();
	printf("After first fork (pid = %d, ppid =%d)\n", getpid(), getppid());

	fork();
	printf("After second fork (pid = %d, ppid =%d)\n", getpid(), getppid());

	fork();
	printf("After third fork (pid = %d, ppid =%d)\n", getpid(), getppid());

	return 0;
}
