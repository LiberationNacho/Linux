#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>
#include <math.h>

#define ARRAY_SIZE 100
#define NUM_READERS 20
#define NUM_WRITERS 5

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t readers_proceed;
    pthread_cond_t writers_proceed;
    int pending_writers;
    int readers;
    int writers;
} RWLock;

RWLock rwlock;
int shared_array[ARRAY_SIZE];
long read_times[NUM_READERS];
long write_times[NUM_WRITERS];

// RWLock 초기화 함수
void rwlock_init(RWLock *rwlock) {
    pthread_mutex_init(&rwlock->mutex, NULL);
    pthread_cond_init(&rwlock->readers_proceed, NULL);
    pthread_cond_init(&rwlock->writers_proceed, NULL);
    rwlock->pending_writers = 0;
    rwlock->readers = 0;
    rwlock->writers = 0;
}

// 읽기 잠금 획득 함수
void rwlock_acquire_read_lock(RWLock *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);
    while (rwlock->pending_writers > 0 || rwlock->writers > 0) {
        pthread_cond_wait(&rwlock->readers_proceed, &rwlock->mutex);
    }
    rwlock->readers++;
    pthread_mutex_unlock(&rwlock->mutex);
}

// 읽기 잠금 해제 함수
void rwlock_release_read_lock(RWLock *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);
    rwlock->readers--;
    if (rwlock->readers == 0 && rwlock->pending_writers > 0) {
        pthread_cond_signal(&rwlock->writers_proceed);
    }
    pthread_mutex_unlock(&rwlock->mutex);
}

// 쓰기 잠금 획득 함수
void rwlock_acquire_write_lock(RWLock *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);
    rwlock->pending_writers++;
    while (rwlock->readers > 0 || rwlock->writers > 0) {
        pthread_cond_wait(&rwlock->writers_proceed, &rwlock->mutex);
    }
    rwlock->pending_writers--;
    rwlock->writers++;
    pthread_mutex_unlock(&rwlock->mutex);
}

// 쓰기 잠금 해제 함수
void rwlock_release_write_lock(RWLock *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);
    rwlock->writers--;
    if (rwlock->pending_writers > 0) {
        pthread_cond_signal(&rwlock->writers_proceed);
    } else {
        pthread_cond_broadcast(&rwlock->readers_proceed);
    }
    pthread_mutex_unlock(&rwlock->mutex);
}

// 읽기 스레드 함수
void* reader(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    rwlock_acquire_read_lock(&rwlock);
    printf("Reader %ld acquired the read lock\n", (long)arg);

    // 공유 자원을 읽는 작업 수행 (복잡한 작업)
    int sum = 0, max = INT_MIN;
    double average = 0.0, variance = 0.0, stddev = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += shared_array[i];
        if (shared_array[i] > max) {
            max = shared_array[i];
        }
    }
    average = (double)sum / ARRAY_SIZE;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        variance += (shared_array[i] - average) * (shared_array[i] - average);
    }
    variance /= ARRAY_SIZE;
    stddev = sqrt(variance);

    rwlock_release_read_lock(&rwlock);
    printf("Reader %ld released the read lock (sum: %d, max: %d, avg: %.2f, stddev: %.2f)\n", (long)arg, sum, max, average, stddev);

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