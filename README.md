# OS-Scheduler  
A CPU Process Schduler with a Memory Managment Unit
It supports 6 scheduling algorithms in addition to memory managment using Buddy Allocation Algorithm.

## Requirements to run the code
1. GCC Compiler
2. Linux Environment: Linux Machine, VM, WSL, or else

## Scheduling Algorithms
1. First Come First Serve FCFS
2. Non-preemptive Highest priority First (HPF)
3. Preemptive Highest priority First (HPF)
4. Shortest Job First (SJF)
5. Shortest Remaining Time Next (SRTN)
6. Round Robin (RR)

## Memory Managment
Memory is managed using the Buddy Allocation Memory Allocation Method, implemented using a binary tree.

## System Description

Clock: Serves as a virtual integer time clock within the system. Its primary functions include simulation purposes and facilitating IPC.

Process Generation Module: Responsible for generating processes, reading input files, initializing the scheduler, and creating the clock process.  

Scheduler: Responsible for managing processes and their states. It implements diverse scheduling algorithms, determining the execution order and duration of each process.

Process: Each process in the system functions as a CPU-bound entity, carrying out computational tasks.  
Upon completion, a process notifies the scheduler to facilitate termination and efficient resource allocation.

## Input/Output Handling:
The operational dynamics of the project rely on input and output files to emulate and assess the design of the operating system. The specified files encompass:

### Input File: processes.txt

The input  source, with each non-comment line representing a distinct process. Fields detail information such as ID, arrival time, runtime, priority, and memory size.

### Output Files:

#### scheduler.log

The scheduler.log file meticulously records the scheduler's activities and the evolving states of processes at various time points.  
Entries encompass crucial details like process initiation, termination, resumption, and completion times, along with process-specific particulars.

#### scheduler.perf

Contained within the scheduler.perf file are comprehensive performance metrics gauging the scheduler's efficiency.  
This includes statistics on CPU utilization, average waiting time, average weighted turnaround time, and the standard deviation for average weighted turnaround time.

##### memory.log

The memory.log file keeps a detailed log of memory allocation and deallocation events.  
Information includes allocated bytes for each process, timestamps for allocation and deallocation, and the corresponding process ID.


## Data Structures Used
A Priority Queue is used to implement all the algorithms.  
To achieve the queue behavior where needed, the priority queue is utilized with all its elements(processes) having a UNIFIED_PRIORITY.

## How to run the code?
1. Download the file and extract it
2. Open your linux shell or a new project in Vscode editor, whichever you are using.
3. Type the two commands: "make" followed by "make run"

## Notes
System Signals and Inter-Process Communication Methods (Message Queues, Shared Memory and Semaphores) are used for synchronization and coordination between the processes in the system.
