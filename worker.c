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

	int durationSeconds = atoi(argv[1]);
	int durationNano = atoi(argv[2]);

	key_t key = ftok(".", 'S');
	int shmid = shmget(key, sizeof(SystemClock), 0666);
	if (shmid == -1) {
		perror("worker: shmget failed");
		return 1;
	}

	SystemClock *shmClock = (SystemClock *)shmat(shmid, NULL, 0);
	if (shmClock == (void *)-1) {
		perror("worker: shmat failed");
		return 1;
	}

	int startSeconds = shmClock -> seconds;
	int startNano = shmClock -> nanoseconds;

	int termSeconds = startSeconds + durationSeconds;
	int termNano = startNano + durationNano;

	if (termNano >= 1000000000) {
		termSeconds++;
		termNano -= 1000000000;
	}

	printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
	printf("SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d\n", shmClock -> seconds, shmClock -> nanoseconds, termSeconds, termNano);
	printf("--Just Starting\n");

	int lastSecond = shmClock -> seconds;
	int secondsPassed = 0;

	while (shmClock -> seconds < termSeconds || (shmClock -> seconds == termSeconds && shmClock -> nanoseconds < termNano)) {

	if (shmClock -> seconds > lastSecond) {
		secondsPassed++;
		printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
		printf("SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d\n", shmClock -> seconds, shmClock -> nanoseconds, termSeconds, termNano);
		printf("--%d seconds have passed since starting\n", secondsPassed);
		lastSecond = shmClock -> seconds;
	}
}

printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
printf("SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d\n", shmClock -> seconds, shmClock -> nanoseconds, termSeconds, termNano);
printf("--Terminating\n");

shmdt(shmClock);
return 0;

}
