#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_READERS 1000 // 읽기 스레드 수

pthread_rwlock_t rwlock;
pthread_mutex_t mutex;
double rwlock_total_time = 0; // RWLock을 사용한 읽기 작업의 총 시간을 저장하는 변수
double mutex_total_time = 0; // Mutex를 사용한 읽기 작업의 총 시간을 저장하는 변수

// 읽기 작업을 수행하는 스레드 함수
void* read_clear_time(void* arg) {
    // RWLock으로 보호된 읽기 작업
    struct timespec start_rw, end_rw;
    clock_gettime(CLOCK_MONOTONIC, &start_rw); // 읽기 작업 시작 시간 기록
    pthread_rwlock_rdlock(&rwlock); // RWLock 읽기 잠금 획득
    // 읽기 작업 시뮬레이션을 위해 임의의 딜레이 추가
    usleep(1000); // 1ms
    pthread_rwlock_unlock(&rwlock); // RWLock 읽기 잠금 해제
    clock_gettime(CLOCK_MONOTONIC, &end_rw); // 읽기 작업 종료 시간 기록
    double rwlock_time = (end_rw.tv_sec - start_rw.tv_sec) + (end_rw.tv_nsec - start_rw.tv_nsec) / 1e9; // RWLock을 사용한 읽기 작업에 소요된 시간 계산
    rwlock_total_time += rwlock_time; // RWLock을 사용한 읽기 작업의 총 시간에 누적

    // Mutex로 보호된 읽기 작업
    struct timespec start_mutex, end_mutex;
    clock_gettime(CLOCK_MONOTONIC, &start_mutex); // 읽기 작업 시작 시간 기록
    pthread_mutex_lock(&mutex); // Mutex 잠금 획득
    // 읽기 작업 시뮬레이션을 위해 임의의 딜레이 추가
    usleep(1000); // 1ms
    pthread_mutex_unlock(&mutex); // Mutex 잠금 해제
    clock_gettime(CLOCK_MONOTONIC, &end_mutex); // 읽기 작업 종료 시간 기록
    double mutex_time = (end_mutex.tv_sec - start_mutex.tv_sec) + (end_mutex.tv_nsec - start_mutex.tv_nsec) / 1e9; // Mutex를 사용한 읽기 작업에 소요된 시간 계산
    mutex_total_time += mutex_time; // Mutex를 사용한 읽기 작업의 총 시간에 누적

    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS];

    // 시드 설정
    srand(time(NULL));

    // RWLock 및 Mutex 초기화
    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    // 읽기 스레드 생성 및 실행
    for (int i = 0; i < NUM_READERS; ++i) {
        pthread_create(&readers[i], NULL, read_clear_time, NULL);
    }

    // 읽기 스레드 종료 대기
    for (int i = 0; i < NUM_READERS; ++i) {
        pthread_join(readers[i], NULL);
    }

    // RWLock과 Mutex를 사용한 읽기 작업의 총 시간 및 평균 시간 출력
    printf("Total Read Time (RWLock): %.6f seconds\n", rwlock_total_time);
    printf("Average Read Time (RWLock): %.6f seconds\n", rwlock_total_time / NUM_READERS);
    printf("Total Read Time (Mutex): %.6f seconds\n", mutex_total_time);
    printf("Average Read Time (Mutex): %.6f seconds\n", mutex_total_time / NUM_READERS);

    // RWLock 및 Mutex 해제
    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}
