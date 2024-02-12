#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <unistd.h>
#include "MyCounter.h" // Header file


queue_t q;
atomic_bool stopSignal = false;
atomic_int primeCounter = 0;

int main(int argc, char *argv[]) {

    int opt;
    int num_threads = NUM_THREADS; // number of threads on thread pool
    int batch_size = BATCH_SIZE; // numbers of numbers in single task to check

    // get values from arguments
    while ((opt = getopt(argc, argv, "t:b:")) != -1) {
        switch (opt) {
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'b':
                batch_size = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-t num_threads] [-b batch_size]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // allocate threads and batch
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *batch = malloc(batch_size * sizeof(int));
    if (threads == NULL || batch == NULL) {
        fprintf(stderr, "Failed to allocate memory for threads or batches\n");
        exit(EXIT_FAILURE);
    }

    int batch_count = 0; // to count numbers in current batch

    init_queue(&q); // initialize queue

    // run worker in each thread
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, (void *) &q) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Read numbers, split them in batches and enqueue
    int num;
    while (scanf("%d", &num) != EOF) {
        batch[batch_count++] = num;
        if (batch_count == batch_size) {
            enqueue(&q, batch, batch_count);
            batch_count = 0;
        }
    }

    // Enqueue any remaining numbers in a partial batch
    if (batch_count > 0) {
        enqueue(&q, batch, batch_count);
    }

    free(batch);

    // Signal worker threads to stop and wait for them to finish
    atomic_store_explicit(&stopSignal, true, memory_order_release);
    pthread_cond_broadcast(&q.cond);
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the final count of primes
    printf("%d total primes.\n", primeCounter);

    free(threads);

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

void init_queue(queue_t *q) {
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void *worker_thread(void *arg) {
    queue_t *q = (queue_t *) arg;

    while (1) {
        // -----------------------LOCK-----------------------
        pthread_mutex_lock(&q->lock);
        while (q->front == NULL && !atomic_load_explicit(&stopSignal, memory_order_acquire)) {
            pthread_cond_wait(&q->cond, &q->lock);
        }

        // grace termination (stop worker only if it's done and got signal)
        if (atomic_load_explicit(&stopSignal, memory_order_acquire) && q->front == NULL) {
            pthread_mutex_unlock(&q->lock);
            break;
        }


        task_t *task = q->front;
        q->front = q->front->next;
        if (q->front == NULL) {
            q->rear = NULL;
        }
        pthread_mutex_unlock(&q->lock);
        // ----------------------UNLOCK-----------------------

        int primes = 0;

        for (int i = 0; i < task->count; ++i) {
            if (isPrime(task->numbers[i])) {
                primes++;
            }
        }
        free(task->numbers);

        free(task);
        atomic_fetch_add(&primeCounter, primes);
    }
    return NULL;
}

void enqueue(queue_t *q, int *numbers, int count) {
    task_t *task = malloc(sizeof(task_t)); // allocate task
    if (!task) {
        perror("Failed to allocate task");
        exit(EXIT_FAILURE);
    }

    task->numbers = malloc(count * sizeof(int)); // allocate array of numbers to check in task
    if (!task->numbers) {
        perror("Failed to allocate task numbers");
        free(task);
        exit(EXIT_FAILURE);
    }
    memcpy(task->numbers, numbers, count * sizeof(int)); // copy numbers from batch to task
    task->count = count;
    task->next = NULL;

    // -----------------------LOCK-----------------------
    pthread_mutex_lock(&q->lock);
    if (q->rear == NULL) {
        q->front = q->rear = task;
    } else {
        q->rear->next = task;
        q->rear = task;
    }
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
    // ----------------------UNLOCK-----------------------
}


task_t *dequeue(queue_t *q) {
    // -----------------------LOCK-----------------------
    pthread_mutex_lock(&q->lock);
    if (q->front == NULL) {
        return NULL; // Queue is empty
    }

    // Retrieve the batch task from the front of the queue
    task_t *task = q->front;
    q->front = q->front->next;

    // If the queue is now empty, reset the rear pointer
    if (q->front == NULL) {
        q->rear = NULL;
    }
    pthread_mutex_unlock(&q->lock);
    // ----------------------UNLOCK-----------------------

    return task; // Return the dequeued batch
}




