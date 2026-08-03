Page 1 of 29
CSOPESY Lecture – Memory Management Instructor: Neil Patrick Del Gallego
Objective
• How OS manages its main memory for multiple processes.
• How to emulate memory management for a multi-tasking OS.
Overview
• There are several kinds of memory management models. We will only discuss:
o Contiguous memory or flat memory model
o Paging
o Demand paging
Basic Principles
• Most OS pre-allocate memory for each process, depending on its relative size and
computational demand. The pre-allocated memory can be w ritten/freed when executing
```
instructions → malloc(), dealloc(), pointers, etc.
```
o Each process have a separate memory space.
o Start address and end address present in process info.
• OS also has a share of its memory space!
• Processes cannot execute without their resources loaded into memory.
An Illustration of Flat Memory Allocation
Page 2 of 29Page 3 of 29
Swapping and the Backing Store
• Swapping is a memory management technique used by operating systems to temporarily move a
```
process or a portion of a process from main memory (RAM) to secondary storage (usually a
```
```
disk) and vice versa. The secondary storage is the backing store.
```
• The primary goal of sw apping is to free up space in the main memory for active processes.
When a process is swapped out, its contents are moved to the secondary storage, allowing
other processes to utilize the freed-up memory.
• Process: When a process is swapped out, all of its contents are transferred to a reserved
```
space on the disk (swap space). The operating system maintains a swapping policy to
```
determine when to swap processes in and out based on priority, demand, or other criteria.
Page 4 of 29
• Performance Impact: While swapping can prevent memory exhaustion and allow more processes
to run, excessive swapping can lead to performance degradation due to increased I/O
operations between main memory and secondary storage.
The flat memory model follows a combination of first-fit and quick-fit approaches. Here are the
primary fit approaches:
1. First Fit:
• Description: In the first-fit approach, the operating system allocates the first
available block of memory that is large enough to accommodate the process's size.
• Advantages: Simple and easy to implement. Quick allocation of memory.
• Disadvantages: May result in fragmentation, both internal and external, over time.
2. Best Fit:
• Description: The best-fit approach allocates the smallest available block of memory
that is large enough to fit the process.
• Advantages: Minimizes wasted memory by selecting the closest match in size. Reduces
external fragmentation.
• Disadvantages: May lead to fragmentation and may require searching through the entire
list of available blocks.
3. Worst Fit:
• Description: The worst-fit approach allocates the largest available block of memory.
It aims to leave the largest holes for future allocations.
• Advantages: Can lead to larger available blocks for subsequent allocations.
• Disadvantages: May result in more wasted memory due to fragmentation. Like best fit,
searching for the largest block can be time -consuming.
4. Next Fit:
• Description: Similar to the first-fit approach, but it starts searching for available
memory from the last allocation point. It continues searching until a suitable block
is found.
• Advantages: Reduces fragmentation compared to first fit. Simplifies implementation.
• Disadvantages: Still susceptible to fragmentation.
5. Quick Fit:
• Description: In quick fit, the memory is divided into predefined block sizes, and
each size has its own list of available blocks. Allocation involves finding the
appropriate size class and selecting the first available block.
• Advantages: Quick allocation, especially for commonly used block sizes.
• Disadvantages: May lead to fragmentation within size classes.
Each approach aims to reduce but cannot eliminate fragmentation.
Page 5 of 29
Emulating a first-fit flat memory model
The necessary condition for a process to be selected for CPU execution is that an available memory
block is available. If it is not available, we can either:
• Select a random process to be put into a backing store
• Put the candidate process back into the ready queue and wait for another chance to use the
memory.
First, consider an OS design with no backing store support and use a first-come-first-serve
scheduler.
Page 6 of 29
Flat memory allocator example
The flat memory allocator is implemented as:
Page 7 of 29Page 8 of 29
Concept of placement of new operator
• To effectively manage memory and have more control over which memory blocks are allocated,
we use “placement new”
• Placement new is a variation new operator in C++.
```
o Normal new operator does two things : (1) Allocates memory (2) Constructs an object
```
in allocated memory.
o Placement new allows us to separate above two things. In placement new, we can pass a
preallocated memory and construct an object in the passed memory.
An example. Allocate 10 uint8_t items in a stack-based order:
Another example. Allocate 10 int32_t items in a stack-based order:
Page 9 of 29
Below shows the sample run of these functions:
Explanation of the uint8_t and int32_t placement new allocation:
```
Allocation:
```
• Allocates a block of memory to store 10 times the size of a uint8_t using new
```
uint8_t[MAX_SIZE * sizeof(uint8_t)].
```
Stack Implementation:
• Uses a uint8_t* pointer named top to keep track of the next available position, mimicking a
stack.
```
Pushing Elements (10 times):
```
• Iterates 10 times.
• Inside the loop:
```
o Uses new (top) uint8_t(200) to construct a uint8_t with value 200 at the memory
```
```
location pointed to by top (placement new).
```
o Increments top to point to the next available position.
```
Printing Elements (LIFO order):
```
• Loops until top reaches the beginning of the allocated memory.
• Inside the loop:
o Decrements top to point to the last element pushed.
o Prints the value of the element at *top after casting it to int for easier
```
visualization (since uint8_t is typically printed as an unsigned char).
```
```
o Prints the memory address of the element using reinterpret_cast<void*>(top).
```
```
Deallocation:
```
• Deallocates the entire memory block using delete[] data.
Page 10 of 29
There may be cases where we may need to reuse the memory allocated. This is where the “placement
new” operator is needed, as there is no need to allocate new memory.
Below is an example of our allocation using 10 int32_t items, which are then replaced by new
items.
```
Explanation:
```
1. Allocation for Signed Integers (int32_t):
o Allocates a block of memory to store 10 times the size of an int32_t using new
```
int32_t[MAX_SIZE * sizeof(int32_t)].
```
o Creates a pointer top of type int32_t* to keep track of the next available position.
2. Pushing Initial Values (10 times):
o Iterates 10 times.
o Inside the loop:
```
▪ Uses new (top) int32_t(15235521) to construct an int32_t with value 15235521
```
```
at the memory location pointed to by top (placement new).
```
▪ Increments top to point to the next available position.
3. Printing Initial Elements (LIFO order):
o Loops until top reaches the beginning of the allocated memory.
o Inside the loop:
▪ Decrements top to point to the last element pushed.
▪ Prints the value of the element at *top after casting it to int for easier
visualization.
```
▪ Prints the memory address of the element using reinterpret_cast<int>(top).
```
4. Re-allocating/Overwriting Data:
o Prints a message indicating re-allocation and overwriting.
5. Overwriting Existing Values (10 times):
o Iterates 10 times again.
o Inside the loop:
```
▪ Uses new (top) int32_t(-100) to construct an int32_t with value -100 at the
```
```
same memory locations used previously (overwriting existing values).
```
```
▪ Increments top to point to the next available position (although it's not
```
```
actually used in this case).
```
6. Printing Overwritten Elements (LIFO order):
o Loops until the top reaches the beginning of the allocated memory.
o Inside the loop:
Page 11 of 29
▪ Decrements top to point to the last overwritten element.
```
▪ Prints the value (now -100) and memory address as before.
```
7. Deallocation:
o Deallocates the entire memory block using delete[] data. This frees the memory even
though the data was overwritten.
Purpose of placement new with OS memory management
```
In C++ (especially in OS code where structures can be found at fixed addresses) , it can be useful
```
to construct an object in memory that is obtained elsewhere. This is accomplished through a
technique known as 'placement new'. For example, say you wanted to create an APIC object at
address 0x09FFF0000, then this snippet of code will use placement new to do the trick:
Device driver buffers:
• Device drivers might pre-allocate buffers for communication with hardware. Placement new
```
could be used to construct data structures (like headers) within these buffers to manage
```
the data.
• Without placement new, N device driver buffers may be placed at different locations in
memory
• Harder to debug, harder to visualize.
Process blocks:
• This is useful for efficiently dividing process memory into its defined components: stack,
heap, symbol table, etc.
Fixed memory address range for user processes:
• We can define a fixed memory block where N processes get stored – the user processs space
vs the OS space.
Purpose of “placement new” in the flat memory model:
• In our previous example, since we’re tracking memory blocks in data structures like vectors
and accessing them via unordered map, “placement new” could be determined for identifying
the “starting points” for each block, or a “single entry point” for the whole memory block.
• “Placement new” could be used for more granular tracking of memory addresses → becomes
```
more manual → pointer to available memory, plus using sizeof() for allocation.
```
```
Activity:
```
• Show a working application emulating the first -fit memory allocator for a main memory with
214 = 16,384 bytes of memory.
o Set the following configuration for your latest OS emulator:
▪ num-cpu 2
▪ scheduler “rr”
▪ quantum-cycles 4
▪ preemptive 1
▪ batch-process-freq 0.25
▪ min-ins 100
▪ max-ins 100
Page 12 of 29
o Set the newest parameters for your OS emulator memory manager:
▪ max-overall-mem 16384
▪ min-mem-per-proc 16
▪ max-mem-per-proc 4096
```
o Each process will have a random amount of memory required: 24 (16) ≤ 𝑀 ≤ 212 (4096 ).
```
```
Note: This is your “min-mem-per-proc” and “max-mem-per-proc” in your MCO2 specs.
```
o Each process stays in the memory during its execution. It will be released from
memory every time it is pre-empted/finished execution.
o If memory is full when a process is scheduled, it will revert back to the tail of the
ready queue.
o For every quantum-cycles, produce a text file with the following information:
▪ Timestamp
▪ Number of processes in memory
▪ Total external fragmentation in KB
▪ An ASCII printout of the memory.
▪ See mockup below:
▪
▪ Explanation: The boundaries indicate the memory address limits. For every
process, print its upper limit, and lower limit, following the prescribed
layout. In this example, there are only 2 processes in memory. Then, there’s
```
external fragmentation of: 12,288 (𝑃1 𝑙𝑜𝑤𝑒𝑟 𝑏𝑜𝑢𝑛𝑑) − 8,192(𝑃2 𝑢𝑝𝑝𝑒𝑟 𝑏𝑜𝑢𝑛𝑑) +
```
```
4096 (𝑃2 𝑙𝑜𝑤𝑒𝑟 𝑏𝑜𝑢𝑛𝑑) − 0 (𝑠𝑡𝑎𝑟𝑡 𝑜𝑓 𝑎𝑑𝑑𝑟𝑒𝑠𝑠 ) = 8,192 bytes.
```
Page 13 of 29
Paging
• Most OS paging schemes.
• This method allows non-contiguous allocation of memory for processes → although internal
fragmentation may still occur.
Recall that a process is stored in memory and divided into subspaces.
Paging Concepts
The subspaces in each processes are referred to as pages.
1. The OS maintains a list of free frames.
2. All processes that arrive have a set number of pages.
3. The OS selects free frame lists to assign the pages and creates the page table.
4. Processes that finishes execution returns the free frames.
This is illustrated in the following diagram:
Page 14 of 29
```
Assumptions:
```
• All pages must be loaded in memory for the program to run.
• All pages must be unloaded after the program finishes.
Page 15 of 29
Written Activities for Page Tracing
Internal fragmentation only happens within the pages themselves but they are very minimal → heap
and stack pre-allocations.
Page 16 of 29
Emulating paging and backing store
Code translations:
• The process, memory allocator, and scheduling classes discussed previously can be extended
to support paging.
• Paging → a process must have defined page numbers. Following the components of a program,
we could design each process to have six pages with the following indices:
Page index Purpose
0 Heap
1 Stack
2 Symbol table
3 Subroutine
4 Libraries
5 Main program
```
• For simplicity, we could limit each page to a fixed sized → placeholder sizeof(char) or
```
```
sizeof(int). We are merely simulating the memory usage of each page.
```
• A class must hold the free frame list and how each frame is mapped to a memory block →
PagingAllocator
• The paging allocator must compute the ideal number of frames to be allocated to a given
memory size → the physical memory block.
Sample code template:
The paging allocator implementation
Page 17 of 29
```
Consider a sample paging scheduler (FCFS scheduler) below that uses the paging allocator:
```
Page 18 of 29
Sample usage:
Page 19 of 29
Backing store operation
To emulate a backing store:
• A backing store could be text files saved in a certain directory. E.g. a backing store
directory relative to your CSOPESY emulator path.
• Why must we save to text files? We must free our main memory usage.
• What should we save in our text files? All necessary process info must be saved , such as:
o Process id, name, command counter
o The number of pages and associated memory sizes, as dictated by its last command
execution. In our case, since process memory and page size is fixed, we simply store
these values.
Backing store architectural concept and real-world alignment
```
• Backing store (or swap space) is typically implemented as a dedicated disk partition (e.g. ,
```
```
Linux swap partition or Windows pagefile.sys)
```
o Real-World OS Strategy: When physical RAM frames run low, the OS memory manager uses
block-layer I/O calls to write victim pages directly to disk sectors. It tracks these
```
pages using a swap map or swap offsets within the Page Table Entry (PTE).
```
o OS Emulator Strategy: In our command-line emulator, we simulate physical RAM using a
centralized byte array or frame pool. To mimic secondary storage transparently, we
serialize paged-out data into a single persistent text file: csopesy-backing-
store.txt.
• Core Challenges in Implementation
o State preservation
▪ When a page is evicted, we must no t only save the raw bytes and variables but
also maintain the context. E.g. process ID, page index that contains the data,
symbol table values, last instruction line, and so on → the OS must be able
to resume the process seamlessly.
o Serialization
```
▪ Values, data types (assuming we support beyond uint16), and different code
```
instructions must be saved in a parseable format.
An example of a structural design of a backing store
The emulator's backing store can be designed using an Indexed Record Architecture. Every time a
page is paged out, it is assigned an address format or text block delimiter in the file.
[PID: 1][PAGE: 0][TYPE: SYMBOL_TABLE]
```
varA:10;varB:5;
```
[PID: 1][PAGE: 1][TYPE: MAIN_PROGRAM]
```
DECLARE varA 10;WRITE 0x500 varA;
```
[PID: 2][PAGE: 0][TYPE: SYMBOL_TABLE]
```
varC:42;
```
Page 20 of 29
Every read operation from this file must increment the “num-paged-in” counter and every write must
increment the “num-page-out” counter, which will be reflected in the vmstat command.
Data structure
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
// Representation of a virtual page's current status
```
struct PageTableEntry {
```
```
int frameNumber = -1; // Maps to physical RAM if valid
```
```
bool isValid = false; // true = in RAM, false = in Backing Store (Page Fault
```
```
target)
```
```
bool isDirty = false; // true = modified since loaded, requires write-back
```
```
};
```
```
// Simulated page data block (can hold variables or instructions)
```
```
struct PageData {
```
```
int pageIndex;
```
```
std::string type; // e.g., "SYMBOL_TABLE", "MAIN_PROGRAM"
```
```
std::string rawContent; // Serialized string representation of the
```
variables/instructions
```
};
```
Other notes
• The movement between the “memory representation” in the running emulator, and to the
```
backing store (csopesy-backing-store.txt) must happen automatically underneath, without
```
intervention from the user.
• Locality of reference – the execution must pause, loop inside a “page fault” phase until
the process state is loaded from the backing store successfully. Instructions are restarted
flawlessly, if necessary.
Page 21 of 29
Demand paging
• To further conserve and have a more efficient memory use, we must ensure that the
process/es selected for CPU execution already reside in our main memory.
• To ensure that we have free frames as much as possible, an alternative strategy is to load
pages only as they are needed – demand paging.
Basic concepts
• The page table is extended to indicate a valid/invalid bit. If the page index is marked as
valid, it means it is in main memory. If it is invalid, it resides in the backing store.
• At the start of execution, an active process will have all its pages in the backing store
and marked as invalid.
• At any point in execution, an active process can have some of its pages put to main memory,
and some put back into a backing store if it’s not needed → another active process can
then load its needed pages into main memory .
Paging instructions and page fault
• If some pages are in a backing store, then this implies that a process may inevitably access
a page that is not yet in main memory.
• Such incident is called a page fault – access to a page marked invalid.
```
The procedure for handling page faults are as follows (see illustration) :
```
Page 22 of 29
1. We check an internal table (page table) for this process to determine whether the reference
was a valid or invalid memory access.
2. If the reference is invalid, we terminate the process (access violation error. This happens
```
in system-level programming languages where memory allocation is explicit). If it was valid
```
but we have not yet brought in that page, we now page it in.
3. We find a free frame.
4. We schedule a secondary storage operation to read the desired page into the newly allocated
frame.
5. When the storage read is complete, we modify the internal table kept with the process and
the page table to indicate that the page is now in memory.
6. We restart the instruction that was interrupted. The process can now access the page as
though it had always been in memory.
Theoretically, some programs could access several new pages of memory with each instruction
```
execution (one page for the instruction and many for data), possibly causing multiple page faults
```
per instruction. This situation would result in unacceptable system performance. Fortunately,
analysis of running processes shows that this behavior is exceedingly unlikely. Programs tend to
have locality of reference which results in
reasonable performance from demand paging.
The hardware to support demand paging is the same as the hardware for
paging and swapping:
• Page table. This table has the ability to mark an entry invalid through a valid –invalid bit or a
special value of protection bits.
• Secondary memory. This memory holds those pages that are not present in the main memory. The
secondary memory is usually a high -speed disk or NVM device. It is known as the swap device, and
the section of storage used for this purpose is known as swap space/backing store.
Page 23 of 29
Maintaining free frames
• Most OS maintain a free-frame list, a pool of free frames for satisfying such
requests.
• Free frames are typically allocated when the stack or heap segments from a process
expand.
• When a system starts up, all available memory is placed on the free -frame list. As
```
free frames are requested (for example, through demand paging), the size of the
```
free-frame list shrinks. At some point, the list either falls to zero or falls
below a certain threshold, at which point it must be repopulated – page
replacement.
Page replacement
• As OS tries to run multiple processes, the degree of multi-programming increases → high
risk and frequency of running out of free frames.
• The operating system has several options at this point:
o Terminate the process
▪ Easy to implement, but not good for the user.
▪ Demand paging is the operating system’s attempt to improve the computer
system’s utilization and throughput.
▪ Users should not be aware that their processes are running on a paged system —
paging should be logically transparent to the user. So, this option is not the
best choice.
o Perform page replacement → if no frame is free, then find a candidate page being
used and free it – but which page?
Page 24 of 29
Page replacement is illustrated below:
The general algorithm for page replacement is as follows:
1. Find the location of the desired page on secondary storage.
2. Find a free frame:
a. If there is a free frame, use it.
b. If there is no free frame, use a page -replacement algorithm to select a victim frame.
c. Write the victim frame to secondary storage; change the page and frame tables
accordingly.
3. Read the desired page into the newly freed frame; change the page and frame tables.
4. Continue the process from where the page fault occurred.
With page replacement, we technically have infinite virtual memory
• Memory seen by the OS and processes isn’t directly tied to the limits of the physical memory.
Page 25 of 29
• Assume that there are no free frames, OS will always find a victim frame to replace
There are different page replacement algorithms:
```
Random:
```
• Principle: Randomly selects a page to evict.
• Pros: Simplicity.
• Cons: Unpredictable and may not perform well in practice.
```
FIFO (First-In-First-Out):
```
• Principle: Evict the oldest page in the memory queue.
• Pros: Simple and easy to implement.
• Cons: May suffer from the "Belady's Anomaly," where increasing the number of frames does not
guarantee a reduction in page faults.
Optimal Page Replacement:
```
• Principle: Evict the page that will not be used for the longest period (optimal, but not
```
```
practically implementable).
```
• Pros: Theoretical optimal solution.
• Cons: Requires future knowledge of page accesses, which is not feasible in a real system.
```
LRU (Least Recently Used):
```
• Principle: Evict the page that has not been used for the longest time.
• Pros: Intuitive and often effective.
• Cons: Implementation complexity. Keeping track of usage timestamps can be resource -intensive.
Page 26 of 29Page 27 of 29
Thrashing
• Consider what occurs if a process does not have “enough” frames—that is, it does not have
the minimum number of frames it needs to support pages in the working set. The process will
quickly page-fault.
• At this point, it must replace some page . However, since all its pages are in active use,
it must replace a page that will be needed again right away.
• Consequently, it quickly faults again, and again, and again, replacing pages that it must
bring back in immediately.
• This high paging activity is called thrashing. A process is thrashing if it is spending
more time paging than executing.
• This incident typically occurs when there are already so many processes running in a
system.
Difference between paging and demand paging
• Paging:
o Divides memory into fixed-sized blocks called pages.
o When a process executes, it splits into pages.
o When the process requests memory, the operating system allocates page frames from the
primary memory.
```
o The operating system moves program pages from the secondary memory (SSD/HDD) to the
```
primary memory frames.
o This technique improves the efficiency of memory management by eliminating external
fragmentation.
• Demand Paging:
o It’s a specific implementation of paging.
o The operating system loads only the necessary pages of a program into memory at
runtime.
o A page fault occurs when the program needs to access a page that is not currently in
memory.
o The operating system then loads the required pages from the disk into memory.
o The operating system updates the page tables accordingly.
o This process is transparent to the running program.
Memory-mapped files and shared libraries
• To further reduce the need for page replacemen t and maximize the efficient use of main
memory – we can dedicate portions of the main memory for shared data.
• Summary: Processes should only have their dedicated memory space used for unique data. Any
data that could be potentially shared must reside only once in the main memory.
Shared libraries:
Page 28 of 29
• System libraries such as the standard C library can be shared by several processes through
mapping of the shared object into a virtual address space.
• Although each process considers the libraries to be part of its virtual address space, the
actual pages where the libraries reside in physical memory are shared by all the processes
```
(Figure 10.3). Typically, a library is mapped read-only into the space of each process that
```
is linked with it.
Memory-mapped files:
• Recall that each process will have some pre -allocated memory.
o Typically the heap/stack is pre-allocated and can run out of space, especially for
operations that perform reading/writing of big datasets.
• Memory-mapped files are a mechanism that enables a file or a portion of a file to be
directly mapped into the virtual memory of a process. This allows the process to access the
file using memory references rather than traditional read and write operations.
• Memory-mapped files are commonly used in scenarios where a large dataset needs to be
accessed in a way that is more efficient than traditional file I/O, such as database
```
systems, certain types of file processing, and shared-memory IPC (Inter-Process
```
```
Communication).
```
All programming languages provide support for memory -mapped files to allow developers to code
applications that would require large files to be read.
Page 29 of 29Page 1 of 12
CSOPESY Lecture – Cooperative and Pre-emptive
Multitasking, Process Synchronization
```
Instructor: Neil Patrick Del Gallego
```
BACKGROUND
• All processes would often require one or more resources. These resources are often limited → number
of CPUs, amount of memory, storage available, I/O devices available.
o At some point in time, there will be resource contention where multiple processes will attempt to
get a certain resource.
```
• CPU cores will simply perform their usual pipelined instructions (fetch, decode, execute) → there must
```
```
be an arbiter (the OS) that decides what set of processes can execute concurrently.
```
• In your OS emulator, there will be lots of processes that need to request the CPU, get memory, and
update reports on CPU/memory utilization → all have possible contention/race conditions .
• There is a need to promote cooperativeness among processes. This is known as process synchronization.
Process synchronization is a fundamental concept in operating systems that coordinates and controls
concurrent processes or threads to ensure orderly execution and prevent conflicts in shared resources. In a
multitasking environment, where multiple processes or threads execute concurrently, it becomes crucial to
synchronize their activities to maintain data consistency, prevent race conditions, and avoid conflicts that could
lead to errors or system instability.
Motivation for Process Synchronization:
1. Shared Resources:
• In a multi-process or multi-threaded environment, processes often share common resources
such as memory, files, and devices. Proper synchronization is necessary to prevent conflicts and
ensure that shared resources are accessed in a controlled manner.
2. Data Consistency:
• Concurrent access to shared data structures can lead to inconsistencies if not synchronized.
Process synchronization ensures that critical sections of code, where shared data is accessed or
modified, are executed atomically.
3. Race Conditions:
• Race conditions occur when the final outcome of a computation depends on the order of
execution of concurrent processes or threads. Synchronization mechanisms help eliminate race
conditions and establish a deterministic order of execution.
4. Deadlock Avoidance:
• Processes waiting for resources may lead to deadlock situations where no progress can be made.
Synchronization techniques, such as deadlock avoidance algorithms, prevent and resolve
deadlock scenarios.
5. Preventing Starvation:
• Fairness in resource allocation is essential to prevent situations where a process or thread is
indefinitely delayed from accessing a resource. Synchronization mechanisms aim to ensure
fairness and prevent starvation.
6. Orderly Communication:
• Processes often communicate with each other through shared data structures or message
passing. Synchronization facilitates orderly communication and prevents data corruption during
inter-process communication.
7. Concurrency Control:
• In database systems, synchronization is crucial for concurrency control to manage multiple
transactions accessing the database simultaneously, ensuring consistency and isolation.
8. System Stability:
• Uncontrolled access to shared resources or lack of synchronization can lead to unpredictable
system behavior, crashes, or data corruption. Process synchronization contributes to overall
system stability and reliability.
In summary, process synchronization is a cornerstone of operating systems, addressing the challenges that
arise in concurrent computing environments. It ensures that multiple processes or threads can coexist and
collaborate effectively without compromising data integrity, system stability, or overall performance.
Synchronization
• Attempting to perform multiple tasks at the same time in an organized manner.
• Developers who create concurrent systems are primarily concerned with:
• Parallelism – Task A and B should execute at the same time to speed up performance.
• Serialization – Task A should execute before Task B.
• Mutual exclusion – Datatype X should only be modified by one task, A or B.
Page 2 of 12
Why concurrency programming is difficult
Consider this sample program with two threads, Alice and Bob.
• First possibility – The computer is parallel. Alice and Bob MAY execute line 2 and 9 at the same time.
In that case it is not easy to know if a statement on one processor is executed before a statement on
another.
• Second possibility – Single processor is running multiple threads of execution. If there are multiple
threads inside a single processor, then the processor can work on one for a while, then switch to
another, and so on.
What is concurrency?
• The developer has no control over when each thread runs. The operating system makes those
```
decisions (depending on the CPU scheduling scheme).
```
• The developer can't tell when statements in different threads will be executed.
• Two events are concurrent if we cannot tell by looking at the program/code which will happen first.
Introduction to Serialization
- Task A should execute before task B.
```
• callBob() passes a message to Bob, which is received by waitForCall(). As long as waitForCall( ) does not
```
```
receive any message from callBob(), it cannot continue executing (thread sleeps).
```
```
• QUESTION: Where should we put waitForCall() to guarantee that Alice calls eatLunch() first?
```
```
Adding waitForCall at line 10 puts the Bob thread to sleep, until callBob() is called by Alice thread. This is
```
called message passing.
Page 3 of 12
Alice thread execution – line A2 < A3 < A4 < A5. where X < Y means task X happens BEFORE task Y. X > Y
means X happens AFTER task Y.
Bob thread execution – line B9 < B10 < B11
With message passing, it guarantees that: A4 < B11.
```
Activity:
```
Are the following statements true/false? Explain why.
• A2 < B9? – False. B9 can happen first.
• B9 < A3? – False. A3 can happen first.
```
• B11 > B10 > A5 > A4? – True. waitForCall() guarantees this.
```
Threads are non-deterministic!
• Concurrent programs are non-deterministic. It is impossible
to tell by looking at the code, which will execute first.
• Non-determinism is one of the things that makes concurrent
programs hard to debug. A program might work correctly
1000 times in a row, and then crash on the 1001st run,
depending on the particular decisions of the scheduler.
```
• These kinds of bugs are almost impossible to find by testing;
```
they can only be avoided by careful programming.
```
Problem: Withdraw-Deposit
```
Consider this problem below.
```
int amount = 0;
```
//deposit
```
class Thread_A {
```
```
void run() {
```
```
amount = amount + 100;
```
```
print(amount);
```
```
}
```
```
}
```
//withdraw
```
class Thread_B {
```
```
void run() {
```
```
amount = amount - 100;
```
```
print(amount);
```
```
}
```
```
}
```
The three essential criteria for solving the critical section problem:
1. Mutual Exclusion - If process Pi is executing in its critical section, then no other processes can be
executing in their critical sections
2. Progress - If no process is executing in its critical section and there exist some processes that wish to
enter their critical section, then the selection of the processes that will enter the critical section next
cannot be postponed indefinitely
3. Bounded Waiting - A bound must exist on the number of times that other processes are allowed to
enter their critical sections after a process has made a request to enter its critical section and before
that request is granted
 Assume that each process executes at a nonzero speed
 No assumption concerning relative speed of the n processes
Page 4 of 12
Peterson’s Solution:
Explanation why it meets all three criteria:
1. Mutual exclusion – If Thread_A is in the critical section, then flag[0] is true. Thread_B will have turn =
0, which causes it to perform busy waiting. Vice versa, if Thread_B is in the critical section, then
flag[1] is true. Thread_A will have turn = 1 which will cause Thread_A to perform busy waiting.
2. Progress – Once Thread_A finishes, it guarantees that Thread_B becomes the next to access the critical
section because flag[0] becomes false, and vice versa.
3. Bounded waiting – Since flag[0]/flag[1] will become false in the future, this guarantees bounded
waiting on the thread executing the while loop.
```
Activity:
```
Consider the pseudocode of Petearson’s solution being used by two threads.
What do you think will happen from the above code?
```
Process synchronization for application-level programs (e.g. games, websites) is discussed on its own in a
```
separate course. E.g. STDISCM /GDPARCM. We are going to discuss process synchronization on the OS level
and how it’s implemented. E.g. How do we represent processes? How do we create our own synchronization
construct?
OPERATING SYSTEM CONTEXT
• The OS is designed to provide synchronization constructs that help developers ensure that their
applications have their threads synchronized as well. E.g. use of mutexes, semaphores, monitors.
• What are the scenarios wherein process synchronization is required, at the OS level?
• The following cases commonly occurs in our day-to-day OS:
1. File Systems:
Page 5 of 12
• In file systems, multiple processes or threads may attempt to access the same file concurrently.
Synchronization mechanisms are implemented to control access to files, preventing conflicts and
ensuring data consistency.
2. Memory Management:
• Memory management involves allocation and deallocation of memory resources. Synchronization
is crucial to manage access to shared memory spaces, preventing race conditions and ensuring
that memory operations occur in a coordinated manner.
3. Device Drivers:
• Device drivers control the communication between the operating system and hardware devices.
Synchronization is essential to coordinate access to device registers and buffers, ensuring that
multiple processes or threads interact with devices in a controlled way.
4. Networking:
• Networking protocols and communication stacks involve concurrent processing of data packets.
Synchronization is implemented to manage access to network buffers, preventing conflicts and
ensuring the proper handling of incoming and outgoing data.
5. Process scheduling:
• The CPU cores and ready queues get updated for every CPU cycle. Thus, information about the
availability of CPU cores and those in the ready queues must be synchronized to avoid re -
scheduling already finished processes or possibly skipping processes in the queue.
Additional support for thread scheduling is provided for application developers:
• In multithreading environments, thread creation, termination, and synchronization require
coordination. Mutexes, semaphores, and condition variables are employed to synchronize threads and
manage shared resources.
```
TODO: Illustration of cooperative multitasking vs pre-emptive multitasking in 2000 operating systems.
```
```
Gist:
```
```
• Cooperative multitasking means that all processes must be programmed (by the developer) to yield
```
control voluntarily
o Problem: A process that spawns processes in an infinite while loop will hog the system and cause
it to crash.
• Pre-emptive multitasking means that the operating system completely manages the scheduling, which
decides when a process should be paused, and another process should be given CPU time.
o This approach allows the operating system to preempt a running process to ensure fair allocation
of CPU resources among all processes.
o Pre-emptive multitasking improves system responsiveness and stability, as no single process can
monopolize the CPU.
• Modern OS employs pre-emptive multitasking.
Consider a basic example of OS scheduling
```
Context:
```
• FCFS scheduler, with only one ready queue, but there are 4 cores that need to be utilized.
• The critical section is the ready queue, where individual schedulers constantly access the ready queue
for getting processes to be executed.
Page 6 of 12
SYNCHRONIZATION CONSTRUCTS
• Synchronization constructs or data structures are made available to application developers.
• OS developers implement the behavior and validate the correctness of these constructs → atomicity,
mutual exclusion guarantee, no race condition on sync constructs themselves.
For CSOPESY, we will only cover one construct, the semaphores. The most basic atomic construct would be
```
the lock, which is simply a boolean flag of 1 (available), 0 (not available).
```
Semaphores
- Data structure that holds N number of permits. Two functions: acquire(S) where S <= N. and
```
release(S).
```
Page 7 of 12
```
Acquire(S)
```
• Attempts to obtain S permits. If
insufficient permits are available,
thread pauses and sleeps at this
line.
• Otherwise, thread proceeds as is and
acquires the permits.
• Note that this process is atomic.
There is no race condition.
```
Release(S)
```
• Releases S amount of permits. Allowing other threads asking for permits
to continue execution.
Declaring a semaphore
Sample Problem: Mutual Exclusion Using Semaphores
Only 1 semaphore is needed.
```
SOLUTION:
```
Sample Problem: Multiplex
Using the solution in problem #1, allow K threads to enter at the critical section at any given point. Assume
there are N threads running. K <= N. For this problem, let N = 10 and K = 5.
```
SOLUTION:
```
Sample Problem: Simple Thread Barrier
Synchronize the threads to satisfy the following relation: a1 < b2 AND b1 < a2. a2 should wait for b1 to
execute and b2 should wait for a1 to execute. This problem is called a thread barrier, where both threads
must meet at a common checkpoint before a2 and b2 executes.
Page 8 of 12
Programming Discussion
Discuss semaphore implementation through C++ mutexes – GDPARCM_HO4 example.
```
Activities: Use semaphores to solve the following problems.
```
1. Consider the Producer-Consumer problem where Producer thread waits for an event. When an event is
detected, it is added into a buffer of MAX SIZE of 10. Consumer thread retrieves the latest event from the
buffer and performs a task related to it. Assume that only 1 producer and 1 consu mer thread is active.
```
Synchronize both threads using semaphores such that waitForEvent() and doSomething() can happen
```
```
concurrently. However, only 1 thread gets to access the buffer (line P6 and C12). Show the pseudocode. TIP:
```
```
You will need to use two semaphores and use BUFFER_SIZE as permit count. (10 pts)
```
```
SOLUTION:
```
2. Show the pseudocode using semaphores if there will be multiple producer and consumer threads active.
```
TIP: You will need to introduce another semaphore (number of permits you must determine). Hence, 3
```
```
semaphores are required. (10 pts)
```
```
NOTE: Using the above solution, this can still result in a race condition. Multiple producer threads acquiring
```
the prodSem can access the same buffer. Must use mutex semaphore.
Page 9 of 12
How is atomicity coded at the OS level?
1. Atomic Instructions:
• Atomic Operations: Modern processors often provide atomic instructions, such as Compare -and-
```
Swap (CAS), that perform read-modify-write operations in a single, uninterruptible step.
```
• Atomicity Guarantee: These instructions ensure that the operation appears to execute
instantaneously and without interference from other processes or threads.
2. Hardware Support:
```
• Transactional Memory (TM): Some processors provide hardware support for transactional
```
memory, allowing a sequence of memory operations to be treated as a single atomic transaction. If
any part of the transaction fails, the changes are rolled back.
```
• Atomic Read-Modify-Write (RMW) Operations: Hardware may support atomic RMW operations
```
like atomic increment, decrement, and test-and-set.
3. Interrupt Disable:
• Disable Interrupts: In certain cases, an OS or kernel might temporarily disable interrupts to
achieve atomicity. This prevents interruption by hardware interrupts during critical sections.
• Caution: Disabling interrupts can have significant implications for system responsiveness and
should be used judiciously.
4. Memory Barriers:
• Memory Barrier Instructions: Memory barriers, or fence instructions, ensure that memory
operations before the barrier are completed before those after the barrier. They help control the
order of memory accesses, enhancing atomicity.
5. Transactional Execution (Hardware Transactional Memory - HTM):
• HTM Support: Some processors support hardware transactional memory, where a block of code
can be executed atomically as a transaction. If conflicts are detected, the transaction is rolled
back.
6. Atomicity in OS Kernels:
• Critical Sections: OS kernels use locks and other synchronization primitives to protect critical
sections where shared data structures are accessed.
• Preemption Control: OS schedulers often include mechanisms to control preemption to avoid
interference during critical operations.
Representing and emulating semaphores
```
What happens to a process if it encounters either acquire() or release() keywords in its instruction set?
```
```
Observation: A correct concurrent program would have its acquire and release system calls on different parts
```
of a process/es. Consider the illustration below:
Page 10 of 12
```
P1 is a sample process with an acquire() step. Thus, a correct synchronized program would have another
```
```
process that would call release() → P2.
```
```
General steps (assume boolean mutex or a 0/1 semaphore):
```
1. When a CPU encountered an acquire() step during execution, it will immediately save the context of
the process and put it back to the ready queue.
2. The CPU scheduler finds a new process to be executed.
3. If the sleeping process is currently selected from the ready queue, it will be skipped . E.g.
```
if(should_execute==false) → return back to RQ.
```
4. One of the processes, containing the release(), would eventually be executed. If this happens, the
```
processes which formerly called acquire() will become eligible for execution again.
```
5. That process, when selected again from the ready queue, will be executed. E.g. if(should_execute) →
put to CPU.
Sometimes, an OS could be designed to hold a sleep backing store, independent from the ready queue, to
provide a more distinguished view of currently sleeping processes. Thus, there would be no condition
checking in the ready queue, since all processes inside it are guaranteed to be ready to execute.
For counting semaphores, the general steps remain the same except a global counter for each declared
semaphore is stored in memory.
Coding guidelines
To simulate semaphore behavior in your CSOPESY emulator, you must introduce a “semaphore declare”
“acquire” and “release” command types in your process class. More details as follows:
Page 11 of 12
```
Specific class (inherits ICommand) Behavior
```
```
SemaphoreDeclare = “semaphore declare” Declares a semaphore and store it in memory, such
```
as a semaphore container.
Acquire Proceeds to acquire flags from a given semaphore,
referencing the semaphore container.
Release Proceeds to release flags from a given container,
referencing the semaphore container.
To ensure atomicity, the semaphore container must be mutually excluded, such as using mutexes before
modifying the flag values.
```
Activity:
```
• Show your initial header files and provide placeholder implementations on how you would support
semaphores in your CSOPESY emulator.
• Using the following test cases below, try emulating this behavior and check the expected output . The
input format is as follows:
•
• The first line indicates how many processes must be declared.
• The input is a space-separated and variable-separated set of strings that follows the format: <PID>
<num commands> <has semaphore> <acquire line> <release line> <num flags> <semaphore to
acquire> <how many> <semaphore to release> <how many>
o PID – the process ID 𝑃𝐼𝐷 < 𝑁, where N is the number of processes.
o <num commands> - Number of commands in the process
o <has semaphore> - 0/1. Indicates if a process has a semaphore. If 1, additional parameters are
provided.
o <acquire line> / <release line> - the line number/command index of where the acquire/release
```
commands are. Command index starts at 0 and ends with (number of total commands – 1).
```
o <num flags> - number of semaphore flags available
o <acquire_pid>/ <release_pid> - which process the semaphore must be acquired/released.
Page 12 of 12
o <acquire_how_many> / <release_how_many> - how many acquires/release of flags must be
performed once the acquire/release command is encountered.
• In the given two sample cases, the first should successfully execute both processes. When the user
types “cpu-util”, the processes should be cleared from the CPU. The second case shows a deadlock. It
should show the both processes indefinitely waiting for each other, indicated by an occupied ready -
queue.