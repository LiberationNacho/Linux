#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_READERS 1000 // 읽기 스레드 수를 늘림

pthread_rwlock_t rwlock;
pthread_mutex_t mutex;
double rwlock_total_time = 0;
double mutex_total_time = 0;

void* read_clear_time(void* arg) {
    // RWLock으로 보호된 읽기 작업
    struct timespec start_rw, end_rw;
    clock_gettime(CLOCK_MONOTONIC, &start_rw);
    pthread_rwlock_rdlock(&rwlock);
    // 읽기 작업 시뮬레이션을 위해 임의의 딜레이 추가
    usleep(1000); // 1ms
    pthread_rwlock_unlock(&rwlock);
    clock_gettime(CLOCK_MONOTONIC, &end_rw);
    double rwlock_time = (end_rw.tv_sec - start_rw.tv_sec) + (end_rw.tv_nsec - start_rw.tv_nsec) / 1e9;
    rwlock_total_time += rwlock_time;

    // Mutex로 보호된 읽기 작업
    struct timespec start_mutex, end_mutex;
    clock_gettime(CLOCK_MONOTONIC, &start_mutex);
    pthread_mutex_lock(&mutex);
    // 읽기 작업 시뮬레이션을 위해 임의의 딜레이 추가
    usleep(1000); // 1ms
    pthread_mutex_unlock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &end_mutex);
    double mutex_time = (end_mutex.tv_sec - start_mutex.tv_sec) + (end_mutex.tv_nsec - start_mutex.tv_nsec) / 1e9;
    mutex_total_time += mutex_time;

    return NULL;
}

void* simulate_game_clear(void* arg) {
    // RWLock으로 보호된 쓰기 작업
    pthread_rwlock_wrlock(&rwlock);
    // 쓰기 작업 시뮬레이션을 위해 임의의 딜레이 추가
    usleep(1000); // 1ms
    pthread_rwlock_unlock(&rwlock);

    // Mutex로 보호된 쓰기 작업
    pthread_mutex_lock(&mutex);
    // 쓰기 작업 시뮬레이션을 위해 임의의 딜레이 추가
    usleep(1000); // 1ms
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS];
    pthread_t game_clearer;

    // 시드 설정
    srand(time(NULL));

    // RWLock 및 Mutex 초기화
    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    // 동시에 여러 클라이언트가 게임 클리어 시간을 읽음
    for (int i = 0; i < NUM_READERS; ++i) {
        pthread_create(&readers[i], NULL, read_clear_time, NULL);
    }

    // 스레드 조인
    for (int i = 0; i < NUM_READERS; ++i) {
        pthread_join(readers[i], NULL);
    }

    printf("Total Read Time (RWLock): %.6f seconds\n", rwlock_total_time);
    printf("Average Read Time (RWLock): %.6f seconds\n", rwlock_total_time / NUM_READERS);
    printf("Total Read Time (Mutex): %.6f seconds\n", mutex_total_time);
    printf("Average Read Time (Mutex): %.6f seconds\n", mutex_total_time / NUM_READERS);

    // RWLock 및 Mutex 해제
    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}

