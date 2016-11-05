#include <stdio.h>
#include <sys/wait.h> // the textbook is wrong
#include <unistd.h>

int main()
{
	pid_t pid = fork();

	if(pid < 0) {
		fprintf(stderr, "Fork() failed");
		return 1;
	} else if(pid == 0) {
		// child
		printf("About to run child, %d\n", pid);
		execlp("/bin/ls", "ls", NULL);
		printf("End child, %d\n", pid); // won't be executed?
	} else {
		// parent
		printf("Starting to wait for child %d\n", pid);
		wait(NULL);
		printf("End waiting for child %d\n", pid);
	}	

	return 0;
}
