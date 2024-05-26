#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_RANKINGS 10
#define NUM_READ_THREADS 5
#define NUM_WRITE_THREADS 5
#define NUM_THREADS (NUM_READ_THREADS + NUM_WRITE_THREADS)

int rankings[NUM_RANKINGS];
pthread_rwlock_t rwlock;
pthread_mutex_t mutex;

double total_rwlock_read_time = 0.0;
double total_rwlock_write_time = 0.0;
double total_mutex_read_time = 0.0;
double total_mutex_write_time = 0.0;

void* read_rankings_rwlock(void* arg);
void* write_rankings_rwlock(void* arg);
void* read_rankings_mutex(void* arg);
void* write_rankings_mutex(void* arg);

void initialize_rankings();
void print_rankings();

void initialize_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        rankings[i] = rand() % 100;
    }
}

void print_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        printf("%d ", rankings[i]);
    }
    printf("\n");
}

double get_elapsed_time(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1e9;
}

void* read_rankings_rwlock(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_rwlock_rdlock(&rwlock);
    printf("RWLock Read: ");
    print_rankings();
    pthread_rwlock_unlock(&rwlock);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_elapsed_time(start, end);
    return (void*)elapsed;
}

void* write_rankings_rwlock(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_rwlock_wrlock(&rwlock);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    printf("RWLock Write: Updated index %d with value %d\n", index, value);
    pthread_rwlock_unlock(&rwlock);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_elapsed_time(start, end);
    return (void*)elapsed;
}

void* read_rankings_mutex(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_lock(&mutex);
    printf("Mutex Read: ");
    print_rankings();
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_elapsed_time(start, end);
    return (void*)elapsed;
}

void* write_rankings_mutex(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_lock(&mutex);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    printf("Mutex Write: Updated index %d with value %d\n", index, value);
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_elapsed_time(start, end);
    return (void*)elapsed;
}

int main() {
    srand(time(NULL));
    initialize_rankings();

    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[NUM_THREADS];

    // RWLock 사용
    printf("Using RWLock:\n");
    for (int i = 0; i < NUM_READ_THREADS; i++) {
        pthread_create(&threads[i], NULL, read_rankings_rwlock, NULL);
    }
    for (int i = NUM_READ_THREADS; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, write_rankings_rwlock, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        void* elapsed;
        pthread_join(threads[i], &elapsed);
        if (i < NUM_READ_THREADS) {
            total_rwlock_read_time += *((double*)elapsed);
        } else {
            total_rwlock_write_time += *((double*)elapsed);
        }
    }

    printf("Average RWLock Read Time: %.6f seconds\n", total_rwlock_read_time / NUM_READ_THREADS);
    printf("Average RWLock Write Time: %.6f seconds\n", total_rwlock_write_time / NUM_WRITE_THREADS);

    // Mutex 사용
    printf("\nUsing Mutex:\n");
    for (int i = 0; i < NUM_READ_THREADS; i++) {
        pthread_create(&threads[i], NULL, read_rankings_mutex, NULL);
    }
    for (int i = NUM_READ_THREADS; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, write_rankings_mutex, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        void* elapsed;
        pthread_join(threads[i], &elapsed);
        if (i < NUM_READ_THREADS) {
            total_mutex_read_time += *((double*)elapsed);
        } else {
            total_mutex_write_time += *((double*)elapsed);
        }
    }

    printf("Average Mutex Read Time: %.6f seconds\n", total_mutex_read_time / NUM_READ_THREADS);
    printf("Average Mutex Write Time: %.6f seconds\n", total_mutex_write_time / NUM_WRITE_THREADS);

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}