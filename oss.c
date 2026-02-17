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

void print_help(char *prog) {
	printf("Usage: %s [-h] [-n proc] [-s simul] [-t timelimit] [-i interval]\n", prog);
	printf("  -h	Show this help\n");
	printf("  -n proc	Total processes to launch (default: 5)\n");
	printf("  -s simul	Max simultaneos processes (default: 3)\n");
	printf("  -t timelimit	Max time for each child in seconds (default: 1)\n");
	printf("  -i interval	Launch interval in seconds (default: 0.1)\n");
}

int main(int argc, char *argv[]) {
	int n = 5;
	int s = 3;
	double t = 1.0;
	double i = 0.1;
	int opt;

	while ((opt = getopt(argc, argv, "hn:s:t:i:")) != -1) {
		switch (opt) {
			case 'h':
				print_help(argv[0]);
				return 0;
			case 'n':
				n = atoi(optarg);
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 't':
				t = atof(optarg);
				break;
			case 'i':
				i = atof(optarg);
				break;
			default:
				print_help(argv[0]);
				return 1;

		}
	}

	printf("OSS: Starting PID:%d PPID:%d\n", getpid(), getppid());
	printf("Called with:\n");
	printf("-n %d\n", n);
	printf("-s %d\n", s);
	printf("-t %.1f\n", t);
	printf("-i %.1f\n", i);

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
		
		int secs = (int)t;
		int nanos = (int)((t - secs) * 1000000000);

		char sec_str[20], nano_str[20];
		snprintf(sec_str, sizeof(sec_str), "%d", secs);
		snprintf(nano_str, sizeof(nano_str), "%d", nanos);

		execl("./worker", "worker", sec_str, nano_str, NULL);
		perror("execl failed");
		exit(1);
	}

	else {
		printf("OSS: Launched worker PID %d\n", pid);

		for (int j = 0; j < 10; j++) {
			clock -> nanoseconds += 100000000;
			if (clock -> nanoseconds >= 1000000000) {
				clock -> seconds++;
				clock -> nanoseconds -= 1000000000;
			}
		}


		wait(NULL);
		printf("OSS: Worker finished\n");

	}

	shmdt(clock);
	shmctl(shmid, IPC_RMID, NULL);
	printf("OSS: Cleaned up and terminating\n");
	return 0;
}
