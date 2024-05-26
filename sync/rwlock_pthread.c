#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>

// 공유 자원
#define ARRAY_SIZE 100
int shared_array[ARRAY_SIZE];

// RWLock 구조체 정의
typedef struct {
    pthread_rwlock_t lock; // pthread_rwlock_t를 사용하여 읽기-쓰기 잠금을 구현
} RWLock;

// RWLock 초기화 함수
void rwlock_init(RWLock *rwlock) {
    pthread_rwlock_init(&rwlock->lock, NULL);
}

// 읽기 잠금 획득 함수
void rwlock_acquire_read_lock(RWLock *rwlock) {
    pthread_rwlock_rdlock(&rwlock->lock); // 읽기 잠금 획득
}

// 읽기 잠금 해제 함수
void rwlock_release_read_lock(RWLock *rwlock) {
    pthread_rwlock_unlock(&rwlock->lock); // 읽기 잠금 해제
}

// 쓰기 잠금 획득 함수
void rwlock_acquire_write_lock(RWLock *rwlock) {
    pthread_rwlock_wrlock(&rwlock->lock); // 쓰기 잠금 획득
}

// 쓰기 잠금 해제 함수
void rwlock_release_write_lock(RWLock *rwlock) {
    pthread_rwlock_unlock(&rwlock->lock); // 쓰기 잠금 해제
}

// 테스트 코드
RWLock rwlock;
long read_times[20];
long write_times[5];

// 읽기 스레드 함수
void* reader(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    rwlock_acquire_read_lock(&rwlock);
    printf("Reader %ld acquired the read lock\n", (long)arg);

    // 공유 자원을 읽는 작업 수행 (복잡한 작업)
    int sum = 0, max = INT_MIN;
    double average = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += shared_array[i];
        if (shared_array[i] > max) {
            max = shared_array[i];
        }
    }
    average = (double)sum / ARRAY_SIZE;

    rwlock_release_read_lock(&rwlock);
    printf("Reader %ld released the read lock (sum: %d, max: %d, avg: %.2f)\n", (long)arg, sum, max, average);

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    read_times[(long)arg] = micros;
    return NULL;
}

// 쓰기 스레드 함수
void* writer(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    rwlock_acquire_write_lock(&rwlock);
    printf("Writer %ld acquired the write lock\n", (long)arg);

    // 공유 자원을 쓰는 작업 수행 (복잡한 작업)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared_array[i] = rand() % 100;
    }

    // 수정 후 검증 작업
    int check_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        check_sum += shared_array[i];
    }

    rwlock_release_write_lock(&rwlock);
    printf("Writer %ld released the write lock (check_sum: %d)\n", (long)arg, check_sum);

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    write_times[(long)arg] = micros;
    return NULL;
}

int main() {
    pthread_t readers[20], writers[5];
    struct timeval start, end;

    rwlock_init(&rwlock); // RWLock 초기화
    srand(time(NULL));

    // 시작 시간 측정
    gettimeofday(&start, NULL);

    // 20개의 읽기 스레드 생성
    for (long i = 0; i < 20; i++) {
        pthread_create(&readers[i], NULL, reader, (void*)i);
    }

    // 5개의 쓰기 스레드 생성
    for (long i = 0; i < 5; i++) {
        pthread_create(&writers[i], NULL, writer, (void*)i);
    }

    // 읽기 스레드 종료 대기
    for (int i = 0; i < 20; i++) {
        pthread_join(readers[i], NULL);
    }

    // 쓰기 스레드 종료 대기
    for (int i = 0; i < 5; i++) {
        pthread_join(writers[i], NULL);
    }

    // 종료 시간 측정
    gettimeofday(&end, NULL);

    // 실행 시간 계산 및 출력
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    printf("Execution time: %ld seconds and %ld microseconds\n", seconds, micros);

    // 개별 스레드 시간 계산 및 출력
    long total_read_time = 0, total_write_time = 0;
    for (int i = 0; i < 20; i++) {
        total_read_time += read_times[i];
    }
    for (int i = 0; i < 5; i++) {
        total_write_time += write_times[i];
    }
    printf("Total read time: %ld microseconds\n", total_read_time);
    printf("Total write time: %ld microseconds\n", total_write_time);
    printf("Average read time: %ld microseconds\n", total_read_time / 20);
    printf("Average write time: %ld microseconds\n", total_write_time / 5);

    return 0;
}
