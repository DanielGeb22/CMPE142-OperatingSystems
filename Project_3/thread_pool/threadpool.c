/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

// the work queue
static task taskQueue[QUEUE_SIZE];
static int queue_head = 0;
static int queue_tail = 0;
static int queue_count = 0;

// Thread pool
static pthread_t bee[NUMBER_OF_THREADS];

// Synchronization primitives
static pthread_mutex_t queue_mutex;
static sem_t task_sem;

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
    if (queue_count == QUEUE_SIZE) {
        return 1; // Full queue
    }

    taskQueue[queue_tail] = t;
    queue_tail = (queue_tail + 1) % QUEUE_SIZE;
    queue_count++;
    return 0;
}

// remove a task from the queue
task dequeue() 
{
    task t = {NULL, NULL};

    if (queue_count == 0) {
        return t;
    }

    t = taskQueue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_SIZE;
    queue_count--;
    return t;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    while (TRUE) {
        sem_wait(&task_sem); // Wait until there is a task

        pthread_mutex_lock(&queue_mutex);
        task work = dequeue();
        pthread_mutex_unlock(&queue_mutex);

        if (work.function != NULL) {
            execute(work.function, work.data);
        }
    }
    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    task t;
    t.function = somefunction;
    t.data = p;

    pthread_mutex_lock(&queue_mutex);
    int result = enqueue(t);
    pthread_mutex_unlock(&queue_mutex);

    if (result == 0) {
        sem_post(&task_sem); // Notify a new task is available
    }

    return result;
}

// initialize the thread pool
void pool_init(void)
{
    pthread_mutex_init(&queue_mutex, NULL);
    sem_init(&task_sem, 0, 0);

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_create(&bee[i], NULL, worker, NULL);
    }
}

// shutdown the thread pool
void pool_shutdown(void)
{
    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_cancel(bee[i]);
    }

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(bee[i], NULL);
    }

    pthread_mutex_destroy(&queue_mutex);
    sem_destroy(&task_sem);
}
