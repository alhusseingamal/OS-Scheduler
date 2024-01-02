# OS-Scheduler  
A CPU Process Schduler with a Memory Managment Unit
It supports 6 scheduling algorithms in addition to memory managment using Buddy Allocation Algorithm.  
System Signals and Inter-Process Communication (IPC) Methods are used for synchronization and coordination between the processes in the system.  
These include: Message Queues, Shared Memory and Semaphores.

## Scheduling Algorithms
1. ### First Come First Serve FCFS
2. ### Non-preemptive Highest priority First (HPF)
3. ### Preemptive Highest priority First (HPF)
4. ### Shortest Job First (SJF)
5. ### Shortest Remaining Time Next (SRTN)
6. ### Round Robin (RR)

## Implementation Notes
1. Three scheduling algorithms are implemented: HPF, SRTN, and RR.
2. For HPF, it is either preemptive or non-preemptive depending on a simple boolean condition.
3. FCFS is realized by using Non-preemptive HPF and setting the priority criteria to the arrival time.
4. SJF is a non-preemptive version of SRTN

## Memory Managment
Memory is managed using the Buddy Allocation Memory Allocation Method, implemented using a binary tree.  

### Important Notes regarding Memory  
1. The memory unit is an emulation for what would happen in a real system.
2. The total memory size is assumed to be 1024 bytes and the max process size is 256 bytes.
   You can modify this in the memory variables section in the memory files.
4. Since the size of meta data is usually much smaller than the size of the actual process, meta data was ignored here. It can be accounted for easily though.
5. It is assumed that the memory allocated for a process remains constant during its time in the system.
6. The system has a single uni-core CPU.

## System Description

Clock: Serves as a virtual integer time clock within the system. Its primary functions include simulation purposes and facilitating IPC.

Process Generation Module: Responsible for generating processes, reading input files, initializing the scheduler, and creating the clock process.  

Scheduler: Responsible for managing processes and their states. It implements diverse scheduling algorithms, determining the execution order and duration of each process.

Process: Each process in the system functions as a CPU-bound entity, carrying out computational tasks.  
Upon completion, a process notifies the scheduler to facilitate termination and efficient resource allocation.

## Input/Output Handling:
The operational dynamics of the project rely on input and output files to emulate and assess the design of the operating system. The specified files encompass:

The user is prompted to enter the choosen scheduling algorithm and, if needed, the quantum.  

### Input File: 

#### processes.txt
The input  source, with each non-comment line representing a distinct process. Fields detail information such as ID, arrival time, runtime, priority, and memory size.

### Output Files:

#### scheduler.log
The scheduler.log file meticulously records the scheduler's activities and the evolving states of processes at various time points.  
Entries encompass crucial details like process initiation, termination, resumption, and completion times, along with process-specific particulars.

#### scheduler.perf
Contained within the scheduler.perf file are comprehensive performance metrics gauging the scheduler's efficiency.  
This includes statistics on CPU utilization, average waiting time, average weighted turnaround time, and the standard deviation for average weighted turnaround time.

#### memory.log
The memory.log file keeps a detailed log of memory allocation and deallocation events.  
Information includes allocated bytes for each process, timestamps for allocation and deallocation, and the corresponding process ID.  
Zero Memory processes have no footprint on the memory log.

## Data Structures Used
A Priority Queue is used to implement all the algorithms.  
To achieve the queue behavior where needed, the priority queue is utilized with all its elements having the same priority.

## Coding Style
The code exhibits a high degree of extensbility and modularity, with blocks being re-used and shared across different algorithms.  
At a few times, the code could have been shortened, but it was left that way for clarity and portability.  
Variable naming is descriptive, and as such comments were often omitted to avoid condensing the code with unnecessary comments.

## Requirements to run the code
1. GCC Compiler
2. Linux Environment: Linux Machine, VM, WSL, or else

## How to run the code?
1. Download the file and extract it.
2. Open your linux shell or a new project your WSL editor, or whichever you are using.
3. Type those two commands in the shell: "make" followed by "make run".
### More Importantly  
4. If you need a pure scheduler with no regard to memory (infinite memory), either set your memory entries to 0 or make the max memory size sufficiently large.
5. If needed, upload your testcases to the file "processes.txt".
6. Alternatively, you an use the test_generator to generate a random testcase for you.
6. You can play with the max memory size, max process size, etc... by editing them in the memory variables section in the memory file

### Some testcases are provided under the "testcases" section. A Sample run of testcases are shown in the "screenshots" section.
