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

void print_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        printf("%d ", rankings[i]);
    }
    printf("\n");
}

void* read_rankings_rwlock(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_rwlock_rdlock(&rwlock);
    print_rankings();
    pthread_rwlock_unlock(&rwlock);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_time_diff(start, end);
    return (void*)elapsed;
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
    return (void*)elapsed;
}

void* read_rankings_mutex(void* arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_lock(&mutex);
    print_rankings();
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = get_time_diff(start, end);
    return (void*)elapsed;
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
    return (void*)elapsed;
}

int main() {
    srand(time(NULL));
    initialize_rankings();

    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[20];
    double total_rwlock_read_time = 0.0;
    double total_rwlock_write_time = 0.0;
    double total_mutex_read_time = 0.0;
    double total_mutex_write_time = 0.0;
    int iterations = 10000;

    printf("RWLock:\n");
    for (int i = 0; i < iterations; i++) {
        // 이중 포인터를 사용하지 않고 단일 포인터로 변경
        double elapsed_times[10]; // 수정된 부분

        for (int j = 0; j < 5; j++) {
            pthread_create(&threads[j], NULL, read_rankings_rwlock, NULL);
        }
        for (int j = 5; j < 10; j++) {
            pthread_create(&threads[j], NULL, write_rankings_rwlock, NULL);
        }
        for (int j = 0; j < 10; j++) {
            // 반환값 받을 변수 주소를 &elapsed_times[j]로 변경
            pthread_join(threads[j], (void**)&elapsed_times[j]);
        }
        for (int j = 0; j < 5; j++) {
            total_rwlock_read_time += elapsed_times[j];
        }
        for (int j = 5; j < 10; j++) {
            total_rwlock_write_time += elapsed_times[j];
        }
    }

    printf("Average Read Time: %.6f seconds\n", total_rwlock_read_time / (5 * iterations));
    printf("Average Write Time: %.6f seconds\n", total_rwlock_write_time / (5 * iterations));

    printf("\nMutex:\n");
    for (int i = 0; i < iterations; i++) {
        double elapsed_times[10]; // 수정된 부분

        for (int j = 0; j < 5; j++) {
            pthread_create(&threads[j], NULL, read_rankings_mutex, NULL);
        }
        for (int j = 5; j < 10; j++) {
            pthread_create(&threads[j], NULL, write_rankings_mutex, NULL);
        }
        for (int j = 0; j < 10; j++) {
            pthread_join(threads[j], (void**)&elapsed_times[j]);
        }
        for (int j = 0; j < 5; j++) {
            total_mutex_read_time += elapsed_times[j];
        }
        for (int j = 5; j < 10; j++) {
            total_mutex_write_time += elapsed_times[j];
        }
    }

    printf("Average Read Time: %.6f seconds\n", total_mutex_read_time / (5 * iterations));
    printf("Average Write Time: %.6f seconds\n", total_mutex_write_time / (5 * iterations));

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}
