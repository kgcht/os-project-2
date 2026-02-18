# OS Process Table and System Clock Simulator

**Name:** Kayla Gaynor

**Date:** February 18, 2026

**Environmnet:** macOS (Darwin), using gcc compiler and terminal

## Project Description

This project implements an operating system simulator with a process table and shared memory system clock. The simulator consists of two programs:

- **oss**: Operating system simulator that manages child processes using a simulated system clock
- **worker**: Child process that monitors the shared memory clock and terminates after a specified duration

The key difference from Project 1 is that processes now use a **simulated clock** in shared memory instead of real time, and OSS maintains a **Process Control Block (PCB) table** to track all workers.

## How to Compile

'''bash
make

## How to Run

./oss [-h] [-n proc] [-s simul] [-t timelimit] [-i interval]

## Command-Line Options

-h: Display help message

-n proc: Total number of worker processes to launch (default: 5)

-s simul: Maximum number of simultaneous processes (default: 3)

-t timelimit: Maximum simulated time for each worker in seconds (default: 1)

-i interval: Minimum simulated time interval between launches in seconds (default: 0.1)


# Examples:

## Launch 10 workers, max 3 simultaneous, each runs 5 simulated seconds
./oss -n 10 -s 3 -t 5

## Launch 20 workers with 2-second launch intervals
./oss -n 20 -s 5 -t 3 -i 2

## Show help
./oss -h

# How to Clean

make clean

## Key Features

**1. Shared Memory Clock:** System clock with seconds and nanoseconds accessible by all processes

**2. Process Control Block (PCB) Table:** Tracks up to 20 processes with their start times, end times, and PIDs

**3. Simulated Time:** Workers check the shared clock (not real time) to determine when to terminate

**4. Process Table Display:** Prints the complete process table every 0.5 simulated seconds

**5. 60-Second Timeout:** Automatically terminates after 60 real seconds, killing all children and cleaning up

**6. Interval Launching:** Respects minimum time between launching new workers

**7. Proper Cleanup:** Shared memory is properly detached and removed on exit


## Files Included

- **oss.c**: Operating system simulator (parent process)

- **worker.c**: Worker process (child process)

- **Makefile**: Build configuration

- **README.md**: This file


## Implementation Details

**Shared Memory Structure:**

typedef struct {
	int seconds;
	int nanoseconds;
} SystemClock;


**Process Control Block:**

typedef struct {
	int occupied;
	pid_t pid;
	int startSeconds;
	int startNano;
	int endingTimeSeconds;
	int endingTimeNano;
} PCB;

**How it Works:**

1. OSS creates shared memory and initializes the system clock to 0:0.

2. OSS launches workers up to the simultaneous limit

3. Each worker calculates when it should terminate

4. Workers continuously check the clock and print updates each simulated second

5. OSS increments the clock each iteration

6. When workers finish, OSS launches replacements

7. OSS prints the process table every 0.5 simulated seconds

8. After 60 real seconds OR all workers complete, OSS cleans up and terminates.


## AI Assistance Acknowledgment

I utilized Claude AI as a learning tool and coding assistant for this project.

**Key Prompts Used*

"How do I implement shared memory in C?" - Guidance on shmget, shmat, and shared memory operations

"How do I make worker processes check a simulated clock?" - Implementing time-based termination without sleep()

"How do I add a process table?" - Creating and managing the PCB array

"How do I implement interval-based launching?" - Adding time checks between worker launches

- Debugging and fixing compilation errors in oss.c


**How AI Helped**

The AI explained concepts like:

- Shared memory system calls

- Importance of not using sleep() or usleep()

- Process Control Blocks and how OSS tracks worker states

- Managing nanosecond overflow when incrementing the clock

- Non-blocking wait with waitpid(WNOHANG)

I understand how all the components work together and was able to test, debbug, and verify functionality independently. The AI served as an interactive tutor helping me learn process management and shared memory.


## Known Issues

- None currently identified

## Notes

- The program uses a simulated clock, not real time

- Workers print updates every simulated second

- OSS prints the process table every 0.5 simulated seconds

- The 60-second timeout uses real time for safety

- Shared memory is properly cleaned up on exit or timeout
