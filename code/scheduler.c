#include "headers.h"
/********************************************************************
 *                                Global variables                   *
 *********************************************************************/
int msgqid_scheduler_process_generator, shmid_remaining_time_scheduler_process_generator, shmid_arrivalTime_scheduler_process_generator;

int processes_count, algo_type, quantum;
int finished_processes_number = 0;
int total_waiting_time = 0;
int total_turnaround_time = 0;
int total_response_time = 0;
int total_processes_running_time = 0;
int *remaining_time_shared_memory; // shared memory for storing the running process' remaining time
int *arrived_processes; // shared memory indicating for the next process arrival time
int received_processes_count = 0;
int processindex = 0;
int simtime = 0;
int finished = 0;
int actual_quantum = -1;

double TWAIT = 0;
double TTURN = 0;
double TWA = 0;
double *processWTA;

struct PCB *running_process = NULL;
Node *readyQueue = NULL;
Node *waitingQueue = NULL;

FILE *logfile, *perffile, *logfile_mem;

// memory structure
struct Tree memTree;
struct Data new_process_data;
#define SUCCESSFUL_ALLOCATION 0
#define SUCCESSFUL_DEALLOCATION 1

int i = 0; // iterator used in our loops

/********************************************************************
 *                             Functions prototypes                  *
 *********************************************************************/

struct PCB *generateProcess();
void endProcess();
void resumeProcess();
void stopProcess();
void startProcess();

void HPF();
void SRTN();
void RR();

void terminateScheduler();
void computeCPUStatistics();

// Used to synchronize between the arrival of new process and the possibility of preemption
// It guarantees that all newly arriving processes are processed to their right queue before any preemption is done
union Semun semun;
#define SEM_KEY 200
int sem1;

void create_semaphore()
{
    sem1 = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    if (sem1 == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }
    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(sem1, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
}

// A shared memory between the process and the running process storing the remaining time of the currently running process
// The remaining time of the process is loaded/unloaded according to the state the process is going into
// It is decremented at each second that passes while the process is still running
void create_remaining_time_shm()
{
    shmid_remaining_time_scheduler_process_generator = shmget(REMAINING_TIME_SHM_KEY, sizeof(int), 0666 | IPC_CREAT);
    if (shmid_remaining_time_scheduler_process_generator == -1){
        perror("Error in creating shared remaining time memory in scheduler.c!");
        exit(-1);
    }
    else{
        remaining_time_shared_memory = (int *)shmat(shmid_remaining_time_scheduler_process_generator, 0, 0);
    }
}


// A shared memory between the scheduler and the process_generator storing the times of the arriving processes
// It is necessary in order to for the scheduler to know the time of newly arriving processes.
void create_arrival_time_shm()
{
    key_t arrivalkey = ftok("keyfile", 66);
    shmid_arrivalTime_scheduler_process_generator = shmget(arrivalkey, sizeof(int) * 100, 0666 | IPC_CREAT);
    if (shmid_arrivalTime_scheduler_process_generator == -1)
    {
        perror("Error in creating shared memory for the remaining time variable in scheduler.c!");
        exit(-1);
    }else{
        arrived_processes = (int *)shmat(shmid_arrivalTime_scheduler_process_generator, NULL, 0); // here we attach the shared memory to the process of id 1
    }
}

// A message queue between the process_generator and the scheduler
// It is responsible for receiving the arriving processes and their data
void create_message_queue_scheduler_process_generator()
{
    key_t msgqkey_scheduler_process_generator = ftok("keyfile", 20);
    msgqid_scheduler_process_generator = msgget(msgqkey_scheduler_process_generator, 0666 | IPC_CREAT);
    if (msgqid_scheduler_process_generator == -1)
    {
        perror("Error in creating message queue in scheduler.c!");
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    initClk();

    create_remaining_time_shm();
    create_message_queue_scheduler_process_generator();
    create_arrival_time_shm();
    create_semaphore();
    signal(PROCESS_TERMINATION_SIGNAL, endProcess);
    signal(STOP_PROCESS_SIGNAL, stopProcess);


     // Checking and Interpreting the CMD arguments
    if (argc < 2){
        perror("Error in arguments!\n");
        exit(-1);
    }

    processes_count = atoi(argv[0]);
    processWTA = (double *)malloc(processes_count * sizeof(double));
    algo_type = atoi(argv[1]);
    quantum = atoi(argv[2]);

    initializeMemory(&memTree);

    int simtime = getClk();

    // open the scheduler log and perf files
    logfile = fopen("scheduler.log", "w");
    perffile = fopen("scheduler.perf", "w");

    // open memory log for writing and write the file headers
    logfile_mem = fopen("memory.log", "w");
    fprintf(logfile_mem, "#At time x allocated y bytes for process z from i to j\n");

    // memory

    switch (algo_type)
    {   
        case FCFS_ALGORITHM: // FCFS is equivalent to NON_PREEMPTIVE_HPF_ALGORITHM with the priorityCriteria = Process Arrival Time
            RR(); // It can also be treated as RR with quantum = Process Remaining Time
            break;
        case NON_PREEMPTIVE_HPF_ALGORITHM: // We check for preemption
            HPF();
            break;
        case PREEMPTIVE_HPF_ALGORITHM: // We DON'T check for preemption
            HPF();
            break;
        case SJF_ALGORITHM: // SJF is like SRTN with no preemption, so we choose not to preempt
            SRTN();
            break;
        case SRTN_ALGORITHM:
            SRTN();
            break;
        case RR_ALGORITHM:
            RR();
            break;
        default:
            break;
    }
    computeCPUStatistics();
    fclose(logfile_mem);
    fclose(logfile);
    fclose(perffile);
    terminateScheduler();
    // upon termination release the clock resources.
    destroyClk(true);
}

/********************************************************************
 *                     Functions implementation                     *
 * ******************************************************************/
void terminateScheduler()
{
    // terminate all IPC resources: shared memories, message queues and Semaphores
    shmctl(shmid_arrivalTime_scheduler_process_generator, IPC_RMID, NULL);
    shmctl(shmid_remaining_time_scheduler_process_generator, IPC_RMID, NULL);
    
    msgctl(msgqid_scheduler_process_generator, IPC_RMID, (struct msqid_ds *)0);
    semctl(sem1, 0, IPC_RMID, (struct semid_ds *)0);

    // free all pointers
    free(processWTA);
    free(remaining_time_shared_memory);
    free(arrived_processes);
    free(running_process);
    free(readyQueue);
    free(waitingQueue);

    printf("Scheduler terminated!\n");

    exit(0);
}


// this function will generate a process that is received from the process generator through the message queue
struct PCB *generateProcess()
{
    // receiving the process from the message queue
    struct process_input_data msgprocess;
    int received = msgrcv(msgqid_scheduler_process_generator, &msgprocess, sizeof(struct process_input_data), 0, IPC_NOWAIT);

    // creating the process
    if (received == -1)
        return NULL;
    else
    {
        ++received_processes_count;
        // setting the PCB
        struct PCB *newpcb = malloc(sizeof(struct PCB));
        newpcb->id = msgprocess.id;
        newpcb->pid = -1;
        newpcb->arrivalTime = msgprocess.arrivalTime;
        newpcb->runTime = msgprocess.runTime;
        newpcb->priority = msgprocess.priority;
        newpcb->waitingTime = 0;
        newpcb->remainingTime = msgprocess.runTime;
        newpcb->endTime = 0;
        newpcb->startTime = 0;
        newpcb->stopTime = 0;
        newpcb->state = 0;
        newpcb->memory = msgprocess.mem;
        return newpcb;
    }
}

void startProcess()
{
    // write the process remaining time to the shared memory. Initially, Remaining Time = Burst Time
    *remaining_time_shared_memory = running_process->remainingTime;
    running_process->state = 1; // mark as running

    int PID = fork();
    if (PID == -1)
    {
        perror("Error in fork!");
        exit(-1);
    }
    else if (PID == 0){ // child process
        printf("proceess initialized with id = %d\n", running_process->id);
        char *args[] = {"./process.out", NULL};
        execv(args[0], args);
    }
    // parent process continues from here
    running_process->pid = PID;

    running_process->startTime = getClk();
    running_process->waitingTime = getClk() - running_process->arrivalTime;
    // print to the scheduler.log
    fprintf(logfile, "at time %d process %d starts arr %d total %d remain %d wait %d\n", getClk(),
    running_process->id, running_process->arrivalTime, running_process->runTime, running_process->remainingTime, running_process->waitingTime);
}

// NOTE: stopProcess() sets running_process = NULL
void stopProcess()
{
    printf("Stopping Process with PID = %d\n", running_process->pid);
    kill(running_process->pid, STOP_PROCESS_SIGNAL);
    running_process->state = 0;
    running_process->stopTime = getClk();
    // print to the scheduler.log
    fprintf(logfile, "at time %d process %d stops arr %d total %d remain %d wait %d\n", getClk(),
    running_process->id, running_process->arrivalTime, running_process->runTime, running_process->remainingTime, running_process->waitingTime);

    running_process = NULL;
}

void resumeProcess()
{
    printf("Resuming Process with PID = %d\n", running_process->pid);
    *remaining_time_shared_memory = running_process->remainingTime;
    kill(running_process->pid, CONTINUE_PROCESS_SIGNAL);
    running_process->state = 1;
    running_process->stopTime = getClk() - running_process->stopTime;

    // calculating waiting time from the stopped time
    running_process->waitingTime += running_process->stopTime;
    // print to the scheduler log file
    fprintf(logfile, "at time %d process %d resumes arr %d total %d remain %d wait %d\n", getClk(),
    running_process->id, running_process->arrivalTime, running_process->runTime, running_process->remainingTime, running_process->waitingTime);
}

// NOTE: endProcess() sets running_process = NULL
void endProcess()
{
    running_process->state = 0;
    running_process->endTime = getClk();
    running_process->waitingTime = getClk() - running_process->arrivalTime - running_process->runTime;
    if (running_process->waitingTime < 0)
        running_process->waitingTime = 0;

    finished = getClk();

    int TA = running_process->endTime - running_process->arrivalTime;
    // print to the scheduler.log
    fprintf(logfile, "at time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
    getClk(), running_process->id, running_process->arrivalTime, running_process->runTime, running_process->remainingTime,
    running_process->waitingTime, TA, (running_process->endTime - running_process->arrivalTime) / (double)running_process->runTime);

    // calculating the total waiting time, total turnaround time and total weighted average
    total_waiting_time += running_process->waitingTime;
    int TAT = running_process->endTime - running_process->arrivalTime;
    total_turnaround_time += TAT;
    total_response_time += running_process->startTime;
    total_processes_running_time += running_process->runTime;
    finished_processes_number++;

    // calculating the total weighted average
    TWAIT = total_waiting_time / processes_count;
    TTURN = total_turnaround_time / processes_count;
    TWA = total_response_time / processes_count;

    // calculating the process weighted average
    processWTA[processindex] = (running_process->endTime - running_process->arrivalTime) / (double)running_process->runTime;
    processindex++;

    // deallocate the process from the memory
    new_process_data.memorysize = running_process->memory;
    new_process_data.process_id = running_process->id;
    // deallocate the process

    // in case of successful deallocation, print to the log file and check if a waiting process, if any,
    // can now be accomodated in the memory
    if(deallocate(memTree.head, &new_process_data) == SUCCESSFUL_DEALLOCATION){
        fprintf(logfile_mem, "At time %3d freed     %3d bytes for process %3d from %3d to %3d\n",
        getClk(), new_process_data.memorysize, new_process_data.process_id, new_process_data.memorystart,
        new_process_data.memoryend - 1);
    }
    else
    {   
        // Use this condition to avoid the case where a failure is printed when in fact this memory doesn't even exist as the process size = 0 bytes
        if(running_process->memory != 0)
            printf("Failed to deallocate process with ID %d from memory tree.\n", running_process->id);
    }
    running_process = NULL;
}

void computeCPUStatistics()
{
    // printing cpu utilization, avg WTA, avg waiting time,stdev WTA in scheduler.perf
    fprintf(perffile, "CPU utilization=%.2f%%\n", (double)total_processes_running_time / (double)finished * 100);
    fprintf(perffile, "Avg WTA=%.2f\n", TTURN);
    printf("waiting time = %d\n", total_waiting_time);
    printf("Number of Processes = %d\n", processes_count);
    fprintf(perffile, "Avg Waiting=%.2f\n", total_waiting_time / (double)processes_count);
    double sum = 0;
    int i = 0;
    for (; i < processes_count; i++)
    {
        sum += processWTA[i];
    }
    double mean = sum / processes_count;
    sum = 0;
    i = 0;
    for (; i < processes_count; i++)
    {
        sum += pow(processWTA[i] - mean, 2);
    }
    double variance = sum / processes_count;
    double std = sqrt(variance);
    fprintf(perffile, "std WTA=%.2f\n", std);
}


int checkMemoryForAllocation(struct PCB *process){
    if(process->memory == 0) return 1;
    struct BTnode *bestnode = NULL;
    search(memTree.head, process->memory, &bestnode);
    if(bestnode){ 
        new_process_data.memorysize = process->memory;
        new_process_data.process_id = process->id;
        int flag = allocate(bestnode, &new_process_data);
        if(flag == SUCCESSFUL_ALLOCATION){
            fprintf(logfile_mem, "At time %3d allocated %3d bytes for process %3d from %3d to %3d\n",
            getClk(), new_process_data.memorysize, new_process_data.process_id, new_process_data.memorystart,
            new_process_data.memoryend - 1);
        }
        else{
            printf("Allocation unsuccessful for process: %d\n", process->id);
            return 0;
        }
    }
    else // no failed to find a memory location
    {
        printf("No suitable memory found for process: %d\n", process->id);
        return 0;
    }
    return 1;
}

void CheckMemoryQueue()
{
    // Check if a waiting process can now enter the queue after 
    struct PCB *frontprocess = NULL;
    if(getPQSize(&waitingQueue) > 0)
        frontprocess = peek(&waitingQueue);
    if(frontprocess != NULL)
    {
        if (checkMemoryForAllocation(frontprocess))
        {
            frontprocess = pop(&waitingQueue);
            if(algo_type == NON_PREEMPTIVE_HPF_ALGORITHM || algo_type == PREEMPTIVE_HPF_ALGORITHM)
                push(&readyQueue, frontprocess, frontprocess->priority);
            else if(algo_type == SRTN_ALGORITHM || algo_type == SJF_ALGORITHM)
                push(&readyQueue, frontprocess, frontprocess->remainingTime);
            else if(algo_type == RR_ALGORITHM || algo_type == FCFS_ALGORITHM)
                push(&readyQueue, frontprocess, UNIFIED_PRIORITY);
        }
    }
}

int checkArrivedProcesses()
{   
    while (arrived_processes[getClk()])
    {
        struct PCB *process = generateProcess(); // creating the process
        while (process)
        {
            // decrement the number of processes that will run at the current time
            --arrived_processes[getClk()];

            // determine the priority: process->arrivalTime for FCFS, 
            // process->priority for HPF, process->remainingTime for SRTN and UNIFIED_PRIORITY for RR
            int priorityCriteria = 0;
            if(algo_type == FCFS_ALGORITHM) priorityCriteria = process->arrivalTime;
            else if(algo_type == NON_PREEMPTIVE_HPF_ALGORITHM || algo_type == PREEMPTIVE_HPF_ALGORITHM)  priorityCriteria = process->priority;
            else if(algo_type == SRTN_ALGORITHM || algo_type == SJF_ALGORITHM) priorityCriteria = process->remainingTime;
            else if(algo_type == RR_ALGORITHM /*|| algo_type == FCFS_ALGORITHM*/) priorityCriteria = UNIFIED_PRIORITY;
            if(checkMemoryForAllocation(process))
                push(&readyQueue, process, priorityCriteria);
            else
                push(&waitingQueue, process, priorityCriteria);

            up(sem1); // To allow for preemption
            process = generateProcess();
        }
    }
}


/*
    Determine whether a process should start for the first time or resume
    This is decided based on the process PID.
    If the PID != -1, then the process has run before. resumeProcess function is called. It simply resumes the process.
    Else, the process didn't run before. startProcess function is called. It forks the process and starts it.
*/

void startOrResumeProcess()
{
    running_process = pop(&readyQueue);
    if(algo_type == RR_ALGORITHM)
    {
        actual_quantum = quantum;
        if(running_process->remainingTime < actual_quantum)
            actual_quantum = running_process->remainingTime;
    }
    if(algo_type == FCFS_ALGORITHM)
        actual_quantum = quantum = running_process->remainingTime;

    if (running_process->pid != -1)
        resumeProcess();
    else
        startProcess();
    
}

// At each time increment, check if a process with higher priorityCriteria should preempt the currently running process
void checkPreemption() 
{   
    if(getPQSize(&readyQueue) > 0) 
    {
        struct PCB *frontprocess = peek(&readyQueue);
        int priorityCriteria = 0, tempPriorityCriteria = 0; // priority criteria of the running process and the process atop the ready queue
        
        if(algo_type == PREEMPTIVE_HPF_ALGORITHM) {
            priorityCriteria = running_process->priority;
            tempPriorityCriteria = frontprocess->priority;
        }
        else if(algo_type == SRTN_ALGORITHM)
        {
            priorityCriteria = running_process->remainingTime;
            tempPriorityCriteria = frontprocess->remainingTime;
        } 

        if(tempPriorityCriteria < priorityCriteria)
        { // the process is preempted --> stop it and start/resume the other one
            printf("Process with PID = %d and ID = %d preempted process with PID = %d and ID = %d\n",
            frontprocess->pid, frontprocess->id, running_process->pid, running_process->id);
            push(&readyQueue, running_process, priorityCriteria);
            stopProcess();
            startOrResumeProcess();
        }
        frontprocess = NULL;
    }
}

// Move the time suitable and update the remaining time shared memory, if suitable. Also, check for preemption.
int previousTime = -1;
void runProcess()
{
    if(previousTime == -1)
        previousTime = getClk();
    else
    {
        if(previousTime + 1 == getClk())
        {
            if(arrived_processes[getClk()])
            {
                down(sem1); // wait until all processes that arrive at this time step are enqueued
            }
            ++previousTime;
            --(running_process->remainingTime);
            *remaining_time_shared_memory = running_process->remainingTime;
            if(algo_type == RR_ALGORITHM)
            {
                --actual_quantum;
            }
        }
    }
    // For preemptive scheduling algorithms, check if the currently running process can be preempted
    if(algo_type == PREEMPTIVE_HPF_ALGORITHM || algo_type == SRTN_ALGORITHM)
    {
        checkPreemption();
    }
    if(actual_quantum == 0)
        previousTime = -1;
}


void HPF()
{    
    while (finished_processes_number < processes_count)
    {
        if(!running_process) // if no process is running
        {
            CheckMemoryQueue();
            if (getPQSize(&readyQueue) > 0)
            {
                startOrResumeProcess();
            }
        }
        else
        {
            if(running_process->remainingTime != 0)
            {
                runProcess();
            }
        }  
        // check if there are any processes that arrived at the current clock time and proces them
        checkArrivedProcesses();
    }
}


void SRTN()
{   
    while (finished_processes_number < processes_count)
    {
        if(!running_process) // if no process is running
        {
            CheckMemoryQueue();
            if(getPQSize(&readyQueue) > 0)
            {
                startOrResumeProcess();
            }
        }
        else // if a process is running
        {            
            if(running_process->remainingTime != 0)
            {
                runProcess();
            }
        }
        // check if there are any processes that arrived at the current clock time and proces them
        checkArrivedProcesses();
    }
}


void RR()
{        
    while (finished_processes_number < processes_count)
    {
        if(!running_process) // if no process is running
        {
            CheckMemoryQueue();
            if(getPQSize(&readyQueue) > 0){
                startOrResumeProcess();
            }
        }
        else // if a process is running
        {
            if(running_process->remainingTime != 0)
            {
                if(actual_quantum != 0)
                {
                    runProcess();
                }
                else
                {
                    if(getPQSize(&readyQueue) > 0) // check if at least one other process is in the queue
                    {
                        push(&readyQueue, running_process, UNIFIED_PRIORITY);
                        stopProcess();
                    }
                    else // this is the case where the process didn't end but no other process exists in the queue and so we don't stop it
                    {
                        actual_quantum = quantum;
                        if(running_process->remainingTime < actual_quantum)
                            actual_quantum = running_process->remainingTime;
                    }
                }
            }
        }
        
        // check if there are any processes that should start at the current clock time and push them all into the ready queue
        checkArrivedProcesses();
    }
}
