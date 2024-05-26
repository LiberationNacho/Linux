#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// 랭킹 데이터와 관련된 구조체 및 변수
#define NUM_RANKINGS 10

int rankings[NUM_RANKINGS];

// RWLock과 Mutex 변수
pthread_rwlock_t rwlock;
pthread_mutex_t mutex;

// 함수 프로토타입
void* read_rankings_rwlock(void* arg);
void* write_rankings_rwlock(void* arg);
void* read_rankings_mutex(void* arg);
void* write_rankings_mutex(void* arg);

void initialize_rankings();
void print_rankings();
double get_time_diff(struct timespec start, struct timespec end);

// 랭킹을 초기화하는 함수
void initialize_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        rankings[i] = rand() % 100;
    }
}

// 랭킹을 출력하는 함수
void print_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        printf("%d ", rankings[i]);
    }
    printf("\n");
}

// 시간 차이를 계산하는 함수
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// RWLock을 사용한 읽기 함수
void* read_rankings_rwlock(void* arg) {
    pthread_rwlock_rdlock(&rwlock);
    printf("RWLock Read: ");
    print_rankings();
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

// RWLock을 사용한 쓰기 함수
void* write_rankings_rwlock(void* arg) {
    pthread_rwlock_wrlock(&rwlock);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    printf("RWLock Write: Updated index %d with value %d\n", index, value);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

// Mutex를 사용한 읽기 함수
void* read_rankings_mutex(void* arg) {
    pthread_mutex_lock(&mutex);
    printf("Mutex Read: ");
    print_rankings();
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Mutex를 사용한 쓰기 함수
void* write_rankings_mutex(void* arg) {
    pthread_mutex_lock(&mutex);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    printf("Mutex Write: Updated index %d with value %d\n", index, value);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// 메인 함수
int main() {
    srand(time(NULL));
    initialize_rankings();

    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[10];
    struct timespec start, end;

    // RWLock 사용
    printf("Using RWLock:\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, read_rankings_rwlock, NULL);
    }
    for (int i = 5; i < 10; i++) {
        pthread_create(&threads[i], NULL, write_rankings_rwlock, NULL);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("RWLock time: %.6f seconds\n", get_time_diff(start, end));

    // Mutex 사용
    printf("\nUsing Mutex:\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, read_rankings_mutex, NULL);
    }
    for (int i = 5; i < 10; i++) {
        pthread_create(&threads[i], NULL, write_rankings_mutex, NULL);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Mutex time: %.6f seconds\n", get_time_diff(start, end));

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}

