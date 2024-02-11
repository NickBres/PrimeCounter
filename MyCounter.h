#ifndef MYCOUNTER_H
#define MYCOUNTER_H

#include <stdbool.h>
#include <pthread.h>

#define NUM_THREADS 1 // number of threads in thread pool

#define BATCH_SIZE 100000

typedef struct batch_task {
    int numbers[BATCH_SIZE];  // Array to hold a batch of numbers
    int count;                // Count of numbers in the batch
    struct batch_task* next;  // Pointer to the next batch task
} batch_task_t;

typedef struct task {
    int type; // 0 for single number, 1 for batch
    union {
        int number; // For single number tasks
        struct { // For batch tasks
            int* numbers;
            int count;
        } batch;
    };
    struct task* next;
} task_t;

typedef struct queue {
    task_t* front; // Pointer to the front of the queue
    task_t* rear; // Pointer to the end of the queue
    pthread_mutex_t lock; // Mutex for protecting the queue
    pthread_cond_t cond; // Condition variable for signaling
} queue_t;

void init_queue(queue_t* q);
void* worker_thread(void* arg);
void enqueue(queue_t* q, int number);
task_t* dequeue(queue_t* q);
bool isPrime(int n);
void enqueue_batch(queue_t* q, int* numbers, int count);
batch_task_t* dequeue_batch(queue_t* q);

#endif // MYCOUNTER_H