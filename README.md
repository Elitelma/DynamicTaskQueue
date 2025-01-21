# How It Works
*Producers:* Add tasks to the queue using the enqueue function.
*Consumers:* Process tasks from the queue using the dequeue function.
*Synchronization:* 
- A mutex ensures that only one thread accesses the queue at a time.
- Semaphores manage the availability of tasks and space in the queue.
- Dynamic Memory: Allocates memory for the queue during initialization and releases it upon termination.
*Requirements*
- GCC Compiler
- POSIX Thread Library (pthread)
