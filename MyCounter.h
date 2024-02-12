#ifndef MYCOUNTER_H
#define MYCOUNTER_H

#include <stdbool.h>
#include <pthread.h>

#define NUM_THREADS 8 // number of threads in thread pool

#define BATCH_SIZE 100000 // number of numbers in each task

typedef struct task {
    int* numbers;  // Pointer to an array of numbers in the batch
    int count;     // The number of numbers in the batch
    struct task* next;  // Pointer to the next task in the queue
} task_t;


typedef struct queue {
    task_t* front; // Pointer to the front of the queue
    task_t* rear; // Pointer to the end of the queue
    pthread_mutex_t lock; // Mutex for protecting the queue
    pthread_cond_t cond; // Condition variable for signaling
} queue_t;

void init_queue(queue_t* q);
void* worker_thread(void* arg);
void enqueue(queue_t* q, int* numbers, int count);
task_t* dequeue(queue_t* q);
bool isPrime(int n);

#endif // MYCOUNTER_H