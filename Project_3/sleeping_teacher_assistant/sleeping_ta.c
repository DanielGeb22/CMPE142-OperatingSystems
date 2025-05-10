#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_OF_CHAIRS 3
#define NUM_OF_STUDENTS 10

pthread_mutex_t mutex;
sem_t students_sem;
sem_t ta_sem;

int waiting_students = 0;

void *student_function(void *id);
void *ta_function(void *arg);

int main()
{
    pthread_t ta_thread;
    pthread_t student_threads[NUM_OF_STUDENTS];
    int student_ids[NUM_OF_STUDENTS];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);

    pthread_create(&ta_thread, NULL, ta_function, NULL);

    for (int i = 0; i < NUM_OF_STUDENTS; i++) {
        student_ids[i] = i + 1;
        pthread_create(&student_threads[i], NULL, student_function, (void *)&student_ids[i]);
    }

    for (int i = 0; i < NUM_OF_STUDENTS; i++) {
        pthread_join(student_threads[i], NULL);
    }

    pthread_cancel(ta_thread); // End TA thread after all students are done

    pthread_mutex_destroy(&mutex);
    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);

    return 0;
}

void *student_function(void *id)
{
    int student_id = *(int *)id;
    while (1) {
        sleep(rand() % 5 + 1); // Student is programming

        pthread_mutex_lock(&mutex);

        if (waiting_students < NUM_OF_CHAIRS) {
            waiting_students++;
            printf("Student %d is waiting. Waiting students = %d\n", student_id, waiting_students);
            pthread_mutex_unlock(&mutex);

            sem_post(&students_sem); // Notify TA
            sem_wait(&ta_sem);       // Wait for TA to help

            printf("Student %d is getting help from the TA.\n", student_id);
        } else {
            printf("Student %d found no available chair and will try later.\n", student_id);
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);
}

void *ta_function(void *arg)
{
    while (1) {
        sem_wait(&students_sem); // Wait for a student

        pthread_mutex_lock(&mutex);

        waiting_students--;

        pthread_mutex_unlock(&mutex);

        printf("TA is helping a student.\n");
        sleep(rand() % 3 + 1); // Simulate helping time
        printf("TA finished helping a student.\n");

        sem_post(&ta_sem); // Signal the student that they got help
    }
}
