#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
	int seconds;
	int nanoseconds;
} SystemClock;

int main() {
	printf("OSS: Starting PID:%d\n", getpid());

	key_t key = ftok(".", 'S');
	int shmid = shmget(key, sizeof(SystemClock), IPC_CREAT | 0666);
	SystemClock *clock = (SystemClock *)shmat(shmid, NULL, 0);

	clock -> seconds = 0;
	clock -> nanoseconds = 0;

	printf("OSS: Clock initialized to %d:%d\n", clock -> seconds, clock -> nanoseconds);

	shmdt(clock);
	shmctl(shmid, IPC_RMID, NULL);
	return 0;
}
