#include "headers.h"

int main(int agrc, char *argv[])
{
    printf("Started Process with PID = %d\n", getpid());
    initClk();

    int *remaining_time_shared_memory = NULL; // points to the shared memory location where the process remaining time is stored
    int shmid = shmget(REMAINING_TIME_SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Error in creating shared remaining time memory in scheduler.c!");
        exit(-1);
    }
    else
    {   // we attach the shared memory to the process
        remaining_time_shared_memory = (int *)shmat(shmid, 0, 0);
    }                                    
    int remainingTime = *remaining_time_shared_memory;
    while (remainingTime != 0)
    {       
        if(remainingTime != *remaining_time_shared_memory)
        {   
            --remainingTime;
            printf("remaining time = %d\n", remainingTime);
        }
    }
    printf("Terminating Process with PID = %d\n", getpid());
    kill(getppid(), PROCESS_TERMINATION_SIGNAL);
    destroyClk(false);
    return 0;
}

