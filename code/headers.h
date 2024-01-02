#ifndef HEADERS_H
#define HEADERS_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "my_semaphores.h"
#include <string.h>
#include "priority_queue.h"
#include "memory.h"
#include <limits.h>
#include <math.h>


typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define REMAINING_TIME_SHM_KEY 400

#define MAX_PROCESSES_COUNT 20
#define MAX_ARRIVAL_TIME 100
#define PROCESS_DATA_SIZE 100

#define FCFS_ALGORITHM 1
#define NON_PREEMPTIVE_HPF_ALGORITHM 2
#define PREEMPTIVE_HPF_ALGORITHM 3
#define SJF_ALGORITHM 4
#define SRTN_ALGORITHM 5
#define RR_ALGORITHM 6


#define UNIFIED_PRIORITY 1

#define PROCESS_TERMINATION_SIGNAL SIGUSR1
#define CONTINUE_PROCESS_SIGNAL SIGCONT
#define STOP_PROCESS_SIGNAL SIGSTOP


/*******************************************************************************
*                                   Process                                   *
* *****************************************************************************/
// This is the data read from the user
struct process_input_data
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
    int mem;
};
/*******************************************************************************
 *                                        PCB Table                             *
 *******************************************************************************/
// The Process Control Block
struct PCB
{
    int id;
    int pid;
    int arrivalTime;
    int runTime;
    int priority;
    int waitingTime;
    int remainingTime;
    int startTime;
    int endTime;
    int stopTime;
    int state;
    int memory;
};

// The shared memory storing the clock. It modified by clk and can be accessed by all modules who have a relation with the clk
int *shmaddr;

// A struct used to receive and send messages through it
struct msgbuff
{
    long mtype;
    char mtext[70];
};

int getClk() {  return *shmaddr; }

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * The clock is only an emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

#endif
