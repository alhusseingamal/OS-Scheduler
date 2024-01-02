#include "headers.h"

/********************************************************************
 *                                Global variables                   *
 *********************************************************************/

#define fileName "processes.txt" // change this according to your file name

// AUXILIARY
int i = 0; // iterator used in our loops
int clk = 0; // keeps track of the current clock time
int process_count = 0;

// THE SHARED MEMORY ADDRESS
int *arrived_processes = NULL;

int msgqid_scheduler_process_generator, shmid_arrivalTime_scheduler_process_generator;

char process_data[PROCESS_DATA_SIZE];
struct process_input_data processes[MAX_PROCESSES_COUNT];


void clearResources(int signum){
    // TODO Clears all resources in case of interruption
    // destroy the shared memory and the message queue
    shmctl(shmid_arrivalTime_scheduler_process_generator, IPC_RMID, NULL);
    msgctl(msgqid_scheduler_process_generator, IPC_RMID, NULL);
    printf("Clock terminating!\n");
    exit(0);
}

void readFile()
{ 
    FILE *filePtr = fopen(fileName, "r"); // open the file for reading only   
    if (filePtr == NULL) { // check if the file is opened successfully
        perror("Error in opening the file!");
        exit(-1);
    }
 
    fflush(stdin); // it is always a good practice to flush the buffer before using fgets function
    fgets(process_data, sizeof(process_data), filePtr); // this reads the headers of the processes input file, we don't need them

    // read the processes
    i = 0;
    while (fscanf(filePtr, "%d\t%d\t%d\t%d\t%d\n", &processes[i].id, &processes[i].arrivalTime, &processes[i].runTime, &processes[i].priority, &processes[i].mem) != EOF)
    {
        ++arrived_processes[processes[i].arrivalTime];
        ++i;
    }
    process_count = i;
    fclose(filePtr);
}

// in this file we will create the processes and send the remaining time to the scheduler
int main(int argc, char * argv[]){

    signal(SIGINT, clearResources); // bind the interrupt signal to our own signal handler called clearResources

    //we need to create a shared memory to be used by the process of id 1 to write the remaining time in it
    //and we need to create a message queue to be used by the process of id 1 to send the remaining time to the scheduler


    key_t arrivalkey = ftok("keyfile", 66); // here we create the key for the shared memory

    // the arrival time of the process will be written in the shared memory in form of string
    // here we create the shared memory that will be used by the process of id 1 to write the arrival time in it
    shmid_arrivalTime_scheduler_process_generator = shmget(arrivalkey, sizeof(int) * MAX_ARRIVAL_TIME, 0666|IPC_CREAT);
    

    // creating a array of arrived processes
    
    // here we attach the shared memory to the process of id 1
    arrived_processes = (int*) shmat(shmid_arrivalTime_scheduler_process_generator , 0, 0);
    
    for (; i < MAX_ARRIVAL_TIME; i++) arrived_processes[i] = 0;

    key_t msgqkey_scheduler_process_generator = ftok("keyfile",20 ); // here we create the key for the message queue
    FILE *keyfile = fopen("keyfile", "r"); // here we create the keyfile to be used by the scheduler

    // creating the message queue
    msgqid_scheduler_process_generator = msgget(msgqkey_scheduler_process_generator, 0666 | IPC_CREAT);

    //check if the message queue is created successfully
    if (msgqid_scheduler_process_generator == -1) {
        perror("Error in creating message queue!");
        exit(-1);
    }else{
        printf("message queue created successfully\n");
    }

    // TODO Initialization

    // 1. Read the input files.
    readFile();
    printf("number of processes = %d\n", process_count);

    // For testing purposes, print the data of the processes
    printf("id\tarrival time\trunning time\tpriority\tmem\n");
    i = 0;
    for (; i < process_count; i++) {
        printf("%d\t%d\t%d\t%d\t%d\n", processes[i].id, processes[i].arrivalTime, processes[i].runTime, processes[i].priority, processes[i].mem);
    }

    // 2. Prompt the user for a scheduling algorithm and its parameters, if any.

    int algo_type;
    printf("Choose the scheduling algorithm:\n");
    printf("1-FCFS\n");
    printf("2-Non-preemptive HPF\n");
    printf("3-Preemptive HPF\n");
    printf("4-SJF\n");
    printf("5-SRTN\n");
    printf("6-RR\n");
    scanf("%d", &algo_type);
    int quantum = 0;
    if(algo_type == RR_ALGORITHM){
        printf("Enter the quantum: ");
        scanf("%d", &quantum);
    }   
    
    // 3. Initiate and create the scheduler and clock processes.
    
    // creating the scheduler process

    printf("creating the scheduler process\n");
    int scheduler_id = fork();
    if (scheduler_id == -1) {
        perror("Error in fork!");
        exit(-1);
    }
    else if (scheduler_id == 0) { // child process
        
        // convert scheduling parameters to strings and send them to the scheduler as CMD arguments
        
        char process_count_string[10];
        tostring(process_count_string, process_count);

        char algo_type_string[10];
        tostring(algo_type_string, algo_type);

        char quantum_string[10];
        tostring(quantum_string, quantum);

        // execute the scheduler
        if (algo_type == RR_ALGORITHM){ // RR : send the quantum to the scheduler
            execl( "./scheduler.out", process_count_string, algo_type_string, quantum_string, NULL);
        }
        else{ // Has no quantum ==> equivalent to quantum = 0
            execl( "./scheduler.out", process_count_string, algo_type_string, "0", NULL);
        }
    }

    // creating the clock process

    int clock_id = fork();
    if (clock_id == -1) {
        perror("Error in fork!");
        exit(-1);
    }
    else if (clock_id == 0)  // child process
    {
        execl("./clk.out", "./clk.out", NULL);
    }


    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);

    // TODO Generation Main Loop
    
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.

    // A process' information is sent to the scheduler when its arrival time equals the current clock time
    i = 0;
    while (i < process_count){
        clk = getClk();
        if (clk == processes[i].arrivalTime){
            int issend =  msgsnd(msgqid_scheduler_process_generator, &processes[i], sizeof(struct process_input_data), !IPC_NOWAIT);
            if (issend == -1) {
                perror("Error in sending the message from process generator to scheduler!");
                exit(-1);
            }else{
                printf("message sent successfully\n");
            } 
            i++;
        }
    }
    
    // waiting for the scheduler to terminate
    waitpid(scheduler_id, NULL, 0);

    destroyClk(true);
}

