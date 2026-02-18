#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct {
	int seconds;
	int nanoseconds;
} SystemClock;

typedef struct {
	int occupied;
	pid_t pid;
	int startSeconds;
	int startNano;
	int endingTimeSeconds;
	int endingTimeNano;
} PCB;

#define MAX_PROCESSES 20

int global_shmid = -1;
SystemClock *global_clock = NULL;
PCB global_processTable[MAX_PROCESSES];

void print_help(char *prog) {
	printf("Usage: %s [-h] [-n proc] [-s simul] [-t timelimit] [-i interval]\n", prog);
	printf("  -h	Show this help\n");
	printf("  -n proc	Total processes to launch (default: 5)\n");
	printf("  -s simul	Max simultaneos processes (default: 3)\n");
	printf("  -t timelimit	Max time for each child in seconds (default: 1)\n");
	printf("  -i interval	Launch interval in seconds (default: 0.1)\n");
}

void print_process_table(PCB processTable[], SystemClock *clock) {
	printf("\nOSS PID:%d SysClockS: %d SysclockNano: %d\n", getpid(), clock -> seconds, clock -> nanoseconds);
	printf("Process Table:\n");
	printf("Entry Occupied PID	StartS StartN EndingTimeS EndingTimeNano\n");

	for (int i = 0; i < MAX_PROCESSES; i++) {
		printf("%-5d %-8d %-6d %-6d %-6d %-11d %-14d\n",
			i,
			processTable[i].occupied,
			processTable[i].pid,
			processTable[i].startSeconds,
			processTable[i].startNano,
			processTable[i].endingTimeSeconds,
			processTable[i].endingTimeNano);
	}
	printf("\n");
}

void timeout_handler(int sig) {
	printf("\n\nOSS: 60 seconds elapsed - terminating all processes\n");

	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (global_processTable[i].occupied == 1) {
			printf("OSS: Killing worker PID %d\n", global_processTable[i].pid);
			kill(global_processTable[i].pid, SIGKILL);
			global_processTable[i].occupied = 0;
	}
}

	if (global_clock != NULL) {
		shmdt(global_clock);
	}
	if (global_shmid != -1) {
		shmctl(global_shmid, IPC_RMID, NULL);
	}

	printf("OSS: Timeout - cleaned up and terminating\n");
	exit(0);
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

	global_shmid = shmid;
	global_clock = clock;

	signal(SIGALRM, timeout_handler);
	alarm(60);

	printf("OSS: Clock initialized to %d:%d\n", clock -> seconds, clock -> nanoseconds);

	int total = 0;
	int running = 0;
	
	PCB processTable[MAX_PROCESSES];
	for (int i = 0; i < MAX_PROCESSES; i++) {
		processTable[i].occupied = 0;
		processTable[i].pid = 0;
		processTable[i].startSeconds = 0;
		processTable[i].startNano = 0;
		processTable[i].endingTimeSeconds = 0;
		processTable[i].endingTimeNano = 0;
	}

	for (int i = 0; i < MAX_PROCESSES; i++) {
		global_processTable[i] = processTable[i];
	}

	int lastPrintSecond = 0;

	//Phase 1: Launch initial burst up to simul limit
	while (running < s && total < n) {
		pid_t pid = fork();

	if (pid < 0) {
		perror("fork failed");
		break;
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
		printf("OSS: Launched worker %d (PID %d) at time %d:%d\n", total + 1, pid, clock -> seconds, clock -> nanoseconds);
		
		int slot = -1;
		for (int m = 0; m < MAX_PROCESSES; m++) {
			if (processTable[m].occupied == 0) {
			slot = m;
			break;
		}
	}

	if (slot != -1) {
		processTable[slot].occupied = 1;
		processTable[slot].pid = pid;
		processTable[slot].startSeconds = clock -> seconds;
		processTable[slot].startNano = clock -> nanoseconds;

		int secs = (int)t;
		int nanos = (int)((t - secs) * 1000000000);
		processTable[slot].endingTimeSeconds = clock -> seconds + secs;
		processTable[slot].endingTimeNano = clock -> nanoseconds + nanos;

		if (processTable[slot].endingTimeNano >= 1000000000) {
		  processTable[slot].endingTimeSeconds++;
		  processTable[slot].endingTimeNano -= 1000000000;
	}
}

		global_processTable[slot] = processTable[slot];

		running++;
		total++;
	}
}

	while (total < n || running > 0) {
		clock -> nanoseconds += 10000000;
		if (clock -> nanoseconds >= 1000000000) {
			clock -> seconds++;
			clock -> nanoseconds -= 1000000000;
	}

		
		if (clock -> nanoseconds >= 50000000 && lastPrintSecond != clock -> seconds) {
			print_process_table(processTable, clock);
			lastPrintSecond = clock -> seconds;
		}

		int status;
		pid_t finished = waitpid(-1, &status, WNOHANG);

		if (finished > 0) {
			printf("OSS: Worker PID %d finnished at time %d:%d\n", finished, clock -> seconds, clock -> nanoseconds);
			running--;

		//Clear this worker from thee process table
		for (int k = 0; k < MAX_PROCESSES; k++) {
			if (processTable[k].pid == finished) {
			  processTable[k].occupied = 0;
			  global_processTable[k].occupied = 0;
			  break;
		}
	}

		if (total < n) {
			pid_t pid = fork();

			if (pid < 0) {
				perror("fork failed");
			}
			else if (pid == 0) {
				int secs = (int)t;
				int nanos = (int)((t - secs) * 1000000000);

				char sec_str[20], nano_str[20];
				snprintf(sec_str, sizeof(sec_str), "%d", secs);
				snprintf(nano_str, sizeof(nano_str), "%d", nanos);

				execl(".worker", "worker", sec_str, nano_str, NULL);
				perror("execl failed");
				exit(1);
			}
			else {
				printf("OSS: Launched worker %d (PID %d) at time %d:%d\n", total + 1, pid, clock -> seconds, clock -> nanoseconds);
				
				int slot = -1;
				for (int m = 0; m < MAX_PROCESSES; m++) {
					if (processTable[m].occupied == 0) {
						slot = m;
						break;
					}
				}

				if (slot != -1) {
					processTable[slot].occupied = 1;
					processTable[slot].pid = pid;
					processTable[slot].startSeconds = clock -> seconds;
					processTable[slot].startNano = clock -> nanoseconds;

					int secs = (int)t;
					int nanos = (int)((t - secs) * 1000000000);
					processTable[slot].endingTimeSeconds = clock -> seconds + secs;
					processTable[slot].endingTimeNano = clock -> nanoseconds + nanos;

					if (processTable[slot].endingTimeNano >= 1000000000) {
					  processTable[slot].endingTimeSeconds++;
					  processTable[slot].endingTimeNano -= 1000000000;
					}

					global_processTable[slot] = processTable[slot];
				}

				running++;
				total++;
			}
		}
	}
}

printf("\nOSS: All %d workers completed\n", total);
printf("OSS: Final clock time: %d:%d\n", clock -> seconds, clock -> nanoseconds);


	shmdt(clock);
	shmctl(shmid, IPC_RMID, NULL);
	printf("OSS: Cleaned up and terminating\n");
	return 0;
}
