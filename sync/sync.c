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
double get_time_diff(struct timespec start, struct timespec end);

void initialize_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        rankings[i] = rand() % 100;
    }
}

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void* read_rankings_rwlock(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_rwlock_rdlock(&rwlock);
    clock_gettime(CLOCK_MONOTONIC, &end);
    pthread_rwlock_unlock(&rwlock);
    double elapsed = get_time_diff(start, end);
    printf("RWLock Read Time: %.6f seconds\n", elapsed);
    return NULL;
}

void* write_rankings_rwlock(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_rwlock_wrlock(&rwlock);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    pthread_rwlock_unlock(&rwlock);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_time_diff(start, end);
    printf("RWLock Write Time: %.6f seconds\n", elapsed);
    return NULL;
}

void* read_rankings_mutex(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_time_diff(start, end);
    printf("Mutex Read Time: %.6f seconds\n", elapsed);
    return NULL;
}

void* write_rankings_mutex(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_lock(&mutex);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_time_diff(start, end);
    printf("Mutex Write Time: %.6f seconds\n", elapsed);
    return NULL;
}

int main() {
    srand(time(NULL));
    initialize_rankings();

    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[20];

    int iterations = 10000;

    printf("RWLock:\n");
    for (int i = 0; i < iterations; i++) {
        pthread_create(&threads[0], NULL, read_rankings_rwlock, NULL);
        pthread_create(&threads[1], NULL, read_rankings_rwlock, NULL);
        pthread_create(&threads[2], NULL, read_rankings_rwlock, NULL);
        pthread_create(&threads[3], NULL, read_rankings_rwlock, NULL);
        pthread_create(&threads[4], NULL, read_rankings_rwlock, NULL);
        pthread_create(&threads[5], NULL, write_rankings_rwlock, NULL);
        pthread_create(&threads[6], NULL, write_rankings_rwlock, NULL);
        pthread_create(&threads[7], NULL, write_rankings_rwlock, NULL);
        pthread_create(&threads[8], NULL, write_rankings_rwlock, NULL);
        pthread_create(&threads[9], NULL, write_rankings_rwlock, NULL);

        for (int j = 0; j < 10; j++) {
            pthread_join(threads[j], NULL);
        }
    }

    printf("Mutex:\n");
    for (int i = 0; i < iterations; i++) {
        pthread_create(&threads[0], NULL, read_rankings_mutex, NULL);
        pthread_create(&threads[1], NULL, read_rankings_mutex, NULL);
        pthread_create(&threads[2], NULL, read_rankings_mutex, NULL);
        pthread_create(&threads[3], NULL, read_rankings_mutex, NULL);
        pthread_create(&threads[4], NULL, read_rankings_mutex, NULL);
        pthread_create(&threads[5], NULL, write_rankings_mutex, NULL);
        pthread_create(&threads[6], NULL, write_rankings_mutex, NULL);
        pthread_create(&threads[7], NULL, write_rankings_mutex, NULL);
        pthread_create(&threads[8], NULL, write_rankings_mutex, NULL);
        pthread_create(&threads[9], NULL, write_rankings_mutex, NULL);

        for (int j = 0; j < 10; j++) {
            pthread_join(threads[j], NULL);
        }
    }

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}
