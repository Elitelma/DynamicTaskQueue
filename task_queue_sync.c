#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_THREADS 5
#define POISON_PILL -1 // Special value to indicate termination

// Shared queue
int* task_queue;       // Dynamically allocated queue
int queue_start = 0;
int queue_end = 0;
int tasks_in_queue = 0; // To debug issues with the queue
int queue_capacity;    

// Mutex and semaphores
pthread_mutex_t queue_mutex;
sem_t task_sem; 

// Function to initialize the queue
void initialize_queue(int capacity) {
    queue_capacity = capacity;
    task_queue = (int*)malloc(queue_capacity * sizeof(int));
    if (task_queue == NULL) {
        perror("Failed to allocate memory for the queue");
        exit(EXIT_FAILURE);
    }
}

// Function to destroy the queue
void destroy_queue() {
    free(task_queue);
}

// Function to add a task to the queue
void enqueue(int task) {
    pthread_mutex_lock(&queue_mutex);

    task_queue[queue_end] = task;
    queue_end = (queue_end + 1) % queue_capacity; // Circular queue
    tasks_in_queue++; 

    pthread_mutex_unlock(&queue_mutex);
    sem_post(&task_sem); 
}

// Function to remove a task from the queue
int dequeue() {
    sem_wait(&task_sem); // Wait for a task to be available
    pthread_mutex_lock(&queue_mutex);

    int task = task_queue[queue_start];
    queue_start = (queue_start + 1) % queue_capacity; // Circular queue
    tasks_in_queue--; 

    pthread_mutex_unlock(&queue_mutex);
    return task;
}

// Function for consumer threads to process tasks
void* process_task(void* arg) {
    long thread_id = (long)arg;
    while (1) {
        int task = dequeue(); // Remove a task from the queue

        if (task == POISON_PILL) {
            printf("Thread %ld received poison pill, exiting...\n", thread_id);
            break;
        }

        printf("Thread %ld is processing task %d\n", thread_id, task);
    }
    return NULL;
}

// Produce tasks and add them to the queue
void add_tasks_enqueue(){

    for (int i = 0; i < 600; i++) {
        printf("Adding task %d to the queue\n", i);
        enqueue(i);
    }

    // Add poison pills to signal threads to stop
    for (int i = 0; i < NUM_THREADS; i++) {
        enqueue(POISON_PILL);
    }
}

int main() {
    pthread_t threads[NUM_THREADS];

    int queue_size = 2000; 
    initialize_queue(queue_size);

    pthread_mutex_init(&queue_mutex, NULL);
    sem_init(&task_sem, 0, 0);

    // Create consumer threads
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, process_task, (void*)i);
    }

    add_tasks_enqueue();

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Debugging: check queue state
    if (tasks_in_queue != 0) {
        printf("Warning: tasks still in queue after shutdown: %d\n", tasks_in_queue);
    }

    pthread_mutex_destroy(&queue_mutex);
    sem_destroy(&task_sem);

    destroy_queue();

    printf("All threads finished, program exiting.\n");
    return 0;
}
