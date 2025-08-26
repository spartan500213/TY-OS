#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_SPAR10PIDANCES 454

int main() {
	int pid;
	printf("give a process pid: "); scanf("%d", &pid); printf("\n");
	printf("printing ancestors of pid: %d\n", pid);
	long result = syscall(SYS_SPAR10PIDANCES, pid);
	printf("syscall returned %ld\n", result);
	return 0;
}
