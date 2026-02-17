#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

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

	//Added fork and launch one worker
	pid_t pid = fork();

	if (pid < 0) {
		perror("fork failed");
		return 1;
	}

	else if (pid == 0) {
		execl("./worker", "worker", "5", "500000", NULL);
		perror("execl failed");
		exit(1);
	}

	else {
		printf("OSS: Launched worker PID %d\n", pid);

		for (int i = 0; i < 10; i++) {
			clock -> nanoseconds += 100000000;
			if (clock -> nanoseconds >= 100000000) {
				clock -> seconds++;
				clock -> nanoseconds -= 100000000;
			}
		}

		printf("OSS: Clock now at %d:%d\n", clock -> seconds, clock -> nanoseconds);

		wait(NULL);
		printf("OSS: Worker finished\n");

	}

	shmdt(clock);
	shmctl(shmid, IPC_RMID, NULL);
	printf("OSS: Cleaned up and terminating\n");
	return 0;
}
