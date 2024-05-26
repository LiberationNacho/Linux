#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_RANKINGS 10

int rankings[NUM_RANKINGS];
pthread_rwlock_t rwlock;
pthread_mutex_t mutex;

void* read_rankings_rwlock(void* arg);
void* write_rankings_rwlock(void* arg);
void* read_rankings_mutex(void* arg);
void* write_rankings_mutex(void* arg);

void initialize_rankings();
void print_rankings();
double get_time_diff(struct timespec start, struct timespec end);

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

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void* read_rankings_rwlock(void* arg) {
    pthread_rwlock_rdlock(&rwlock);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void* write_rankings_rwlock(void* arg) {
    pthread_rwlock_wrlock(&rwlock);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void* read_rankings_mutex(void* arg) {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* write_rankings_mutex(void* arg) {
    pthread_mutex_lock(&mutex);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    srand(time(NULL));
    initialize_rankings();

    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[10];
    struct timespec start, end;

    double total_rwlock_time = 0.0;
    double total_mutex_time = 0.0;
    int iterations = 10000;

    for (int i = 0; i < iterations; i++) {
        // RWLock 사용
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int j = 0; j < 5; j++) {
            pthread_create(&threads[j], NULL, read_rankings_rwlock, NULL);
        }
        for (int j = 5; j < 10; j++) {
            pthread_create(&threads[j], NULL, write_rankings_rwlock, NULL);
        }
        for (int j = 0; j < 10; j++) {
            pthread_join(threads[j], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_rwlock_time += get_time_diff(start, end);

        // Mutex 사용
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int j = 0; j < 5; j++) {
            pthread_create(&threads[j], NULL, read_rankings_mutex, NULL);
        }
        for (int j = 5; j < 10; j++) {
            pthread_create(&threads[j], NULL, write_rankings_mutex, NULL);
        }
        for (int j = 0; j < 10; j++) {
            pthread_join(threads[j], NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_mutex_time += get_time_diff(start, end);
    }

    printf("RWLock time: %.6f seconds\n", total_rwlock_time);
    printf("Mutex time: %.6f seconds\n", total_mutex_time);

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}
