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

## Data Structures Used
A Priority Queue is used to implement all the algorithms.  
To achieve the queue behavior where needed, the priority queue is utilized with all its elements(processes) having a UNIFIED_PRIORITY.

## How to run the code?
1. Download the file and extract it
2. Open your linux shell or a new project in Vscode editor, whichever you are using.
3. type the two commands: "make" followed by "make run"

## Notes
System Signals and Inter-Process Communication Methods (Message Queues, Shared Memory and Semaphores) are used all throughout
for synchronization and coordination between the processes in the system.
