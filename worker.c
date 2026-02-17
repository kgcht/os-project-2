#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
	int seconds;
	int nanoseconds;
} SystemClock;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <seconds> <nanoseconds>\n", argv[0]);
		return 1;
	}

	printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());

	key_t key = ftok(".", 'S');
	int shmid = shmget(key, sizeof(SystemClock), 0666);
	SystemClock *clock = (SystemClock *)shmat(shmid, NULL, 0);

	printf("Worker: Clock shows %d:%d\n", clock -> seconds, clock -> nanoseconds);

	shmdt(clock);
	return 0;
}
