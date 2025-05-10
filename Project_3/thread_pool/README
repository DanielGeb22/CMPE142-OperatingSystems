# Thread Pool Implementation | CMPE 142 - Operating Systems
Project involving the creation and management of a thread pool using Pthreads and POSIX synchronization

## Description

This thread pool implementation initializes a fixed number of worker threads at startup, allowing tasks to be submitted dynamically through a shared queue. When a task is submitted using pool_submit(), it is safely added to a circular queue protected by a mutex lock, and a semaphore is posted to notify that new work is available. Worker threads continuously wait on the semaphore, and once notified, they lock the queue, retrieve a task, and execute it. Synchronization ensures that multiple threads can safely access the queue without conflicts. When the system is shut down through pool_shutdown(), all worker threads are canceled and joined to ensure a clean termination, and the synchronization primitives are properly destroyed. This design allows for efficient concurrent task execution with safe thread communication and coordination.

## Output

Upon compiling and running the program, the resulting array is sorted:

![](threadpooloutput.png)

## References
- Operating System Concepts by Abraham Silberschatz, Peter B. Galvin, and Greg Gagne