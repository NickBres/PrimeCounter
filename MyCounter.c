#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include "MyCounter.h" // Header file


queue_t q;
pthread_mutex_t primeCounterMutex = PTHREAD_MUTEX_INITIALIZER;
volatile int stopSignal = 0;
atomic_int primeCounter = ATOMIC_VAR_INIT(0);

int main() {
    pthread_t threads[NUM_THREADS];
    int batch[BATCH_SIZE];
    int batch_count = 0;

    init_queue(&q);

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, (void*)&q) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Reading numbers and enqueuing them in batches
    int num;
    while (scanf("%d", &num) != EOF) {
        batch[batch_count++] = num;
        if (batch_count == BATCH_SIZE) {
            enqueue_batch(&q, batch, BATCH_SIZE);
            batch_count = 0;
        }
    }

    // Enqueue any remaining numbers in a partial batch
    if (batch_count > 0) {
        enqueue_batch(&q, batch, batch_count);
    }

    // Signal worker threads to stop and wait for them to finish
    stopSignal = 1;
    pthread_cond_broadcast(&q.cond);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the final count of primes
    printf("%d total primes.\n", primeCounter);


    return 0;
}



bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

void init_queue(queue_t* q) {
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void* worker_thread(void* arg) {
    queue_t* q = (queue_t*)arg;

    while (!stopSignal) {
        pthread_mutex_lock(&q->lock);
        while (q->front == NULL && !stopSignal) {
            pthread_cond_wait(&q->cond, &q->lock);
        }

        if (stopSignal) {
            pthread_mutex_unlock(&q->lock);
            break;
        }

        task_t* task = q->front;
        q->front = q->front->next;
        if (q->front == NULL) {
            q->rear = NULL;
        }
        pthread_mutex_unlock(&q->lock);

        if (task->type == 1) { // Batch task
            for (int i = 0; i < task->batch.count; ++i) {
                if (isPrime(task->batch.numbers[i])) {
                    atomic_fetch_add(&primeCounter, 1);
                }
            }
            free(task->batch.numbers);
        }
        free(task);
    }
    return NULL;
}




void enqueue(queue_t* q, int number) {
    task_t* t = malloc(sizeof(task_t));
    t->number = number;
    t->next = NULL;

    pthread_mutex_lock(&q->lock);

    if (q->rear == NULL) {
        q->front = q->rear = t;
    } else {
        q->rear->next = t;
        q->rear = t;
    }

    pthread_cond_signal(&q->cond); // Signal a waiting worker thread
    pthread_mutex_unlock(&q->lock);
}

task_t* dequeue(queue_t* q) {
    pthread_mutex_lock(&q->lock);

    if (q->front == NULL) {
        pthread_mutex_unlock(&q->lock);
        return NULL; // Queue is empty
    }

    // Retrieve the task from the front of the queue
    task_t* task = q->front;
    q->front = q->front->next;

    // If the queue is now empty, reset the rear pointer
    if (q->front == NULL) {
        q->rear = NULL;
    }

    pthread_mutex_unlock(&q->lock);
    return task; // Return the dequeued task
}


void enqueue_batch(queue_t* q, int* numbers, int count) {
    task_t* task = malloc(sizeof(task_t));
    if (!task) {
        perror("Failed to allocate task");
        exit(EXIT_FAILURE);
    }

    // Assuming task_t is designed to handle batches
    task->type = 1; // Indicate this is a batch task
    task->batch.numbers = malloc(count * sizeof(int));
    if (!task->batch.numbers) {
        perror("Failed to allocate batch numbers");
        free(task);
        exit(EXIT_FAILURE);
    }
    memcpy(task->batch.numbers, numbers, count * sizeof(int));
    task->batch.count = count;
    task->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->rear == NULL) {
        q->front = q->rear = task;
    } else {
        q->rear->next = task;
        q->rear = task;
    }
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}


batch_task_t* dequeue_batch(queue_t* q) {
    if (q->front == NULL) {
        return NULL; // Queue is empty
    }

    // Retrieve the batch task from the front of the queue
    batch_task_t* batch = (batch_task_t*)q->front;
    q->front = q->front->next;

    // If the queue is now empty, reset the rear pointer
    if (q->front == NULL) {
        q->rear = NULL;
    }

    return batch; // Return the dequeued batch
}




