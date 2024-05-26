#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_READERS 5

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
long write_time;

// RWLock 초기화 함수
void rwlock_init(RWLock *rwlock)
{
    pthread_mutex_init(&rwlock->mutex, NULL);
    pthread_cond_init(&rwlock->readers_proceed, NULL);
    pthread_cond_init(&rwlock->writers_proceed, NULL);
    rwlock->pending_writers = 0;
    rwlock->readers = 0;
    rwlock->writers = 0;
}

// 읽기 잠금 획득 함수
void rwlock_acquire_read_lock(RWLock *rwlock)
{
    pthread_mutex_lock(&rwlock->mutex);
    while (rwlock->pending_writers > 0 || rwlock->writers > 0)
    {
        pthread_cond_wait(&rwlock->readers_proceed, &rwlock->mutex);
    }
    rwlock->readers++;
    pthread_mutex_unlock(&rwlock->mutex);
}

// 읽기 잠금 해제 함수
void rwlock_release_read_lock(RWLock *rwlock)
{
    pthread_mutex_lock(&rwlock->mutex);
    rwlock->readers--;
    if (rwlock->readers == 0 && rwlock->pending_writers > 0)
    {
        pthread_cond_signal(&rwlock->writers_proceed);
    }
    pthread_mutex_unlock(&rwlock->mutex);
}

// 쓰기 잠금 획득 함수
void rwlock_acquire_write_lock(RWLock *rwlock)
{
    pthread_mutex_lock(&rwlock->mutex);
    rwlock->pending_writers++;
    while (rwlock->readers > 0 || rwlock->writers > 0) 
    {
        pthread_cond_wait(&rwlock->writers_proceed, &rwlock->mutex);
    }
    rwlock->pending_writers--;
    rwlock->writers++;
    pthread_mutex_unlock(&rwlock->mutex);
}

// 쓰기 잠금 해제 함수
void rwlock_release_write_lock(RWLock *rwlock)
{
    pthread_mutex_lock(&rwlock->mutex);
    rwlock->writers--;
    if (rwlock->pending_writers > 0)
    {
        pthread_cond_signal(&rwlock->writers_proceed);
    }
    else 
    {
        pthread_cond_broadcast(&rwlock->readers_proceed);
    }
    pthread_mutex_unlock(&rwlock->mutex);
}

// 합계를 계산하는 함수
int calculate_sum(int* array, int start_index, int end_index)
{
    int sum = 0;
    for (int i = start_index; i < end_index; i++)
    {
        sum += array[i];
    }
    return sum;
}

// 최대값을 찾는 함수
int find_max(int* array, int start_index, int end_index)
{
    int max = INT_MIN;
    for (int i = start_index; i < end_index; i++)
    {
        if (array[i] > max)
        {
            max = array[i];
        }
    }
    return max;
}

// 평균을 계산하는 함수
double calculate_average(int* array, int start_index, int end_index)
{
    int sum = calculate_sum(array, start_index, end_index);
    return (double)sum / (end_index - start_index);
}

// 분산을 계산하는 함수
double calculate_variance(int* array, int start_index, int end_index)
{
    double average = calculate_average(array, start_index, end_index);
    double variance = 0.0;
    for (int i = start_index; i < end_index; i++)
    {
        variance += (array[i] - average) * (array[i] - average);
    }
    return variance / (end_index - start_index);
}

// 표준편차를 계산하는 함수
double calculate_stddev(double variance)
{
    return sqrt(variance);
}

// 읽기 스레드 함수(읽기 작업에서는 공유 배열 (shared_array)의 데이터를 읽고 여러 계산을 수행)
void* reader(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int index = *((int*)arg); // 스레드 인덱스

    rwlock_acquire_read_lock(&rwlock); // 읽기 잠금 획득
    printf("Reader %d acquired the read lock\n", index);

    // 각 스레드의 역할에 따라 처리
    switch (index) {
        case 1:
            // 합계 계산
            int sum = calculate_sum(shared_array, 0, ARRAY_SIZE);
            printf("sum(합계): %d\n", sum);
            break;
        case 2:
            // 최대값 계산
            int max = find_max(shared_array, 0, ARRAY_SIZE);
            printf("max(최대값): %d\n", max);
            break;
        case 3:
            // 평균 계산
            double average = calculate_average(shared_array, 0, ARRAY_SIZE);
            printf("average(평균): %.2f\n", average);
            break;
        case 4:
            // 분산 계산
            double variance = calculate_variance(shared_array, 0, ARRAY_SIZE);
            printf("variance(분산): %.2f\n", variance);
            break;
        case 5:
            // 표준편차 계산
            double stddev = calculate_stddev(calculate_variance(shared_array, 0, ARRAY_SIZE));
            printf("standard deviation(표준편차): %.2f\n", stddev);
            break;
        default:
            break;
    }

    rwlock_release_read_lock(&rwlock); // 읽기 잠금 해제
    printf("Reader %d released the read lock\n", index);

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    read_times[index] = micros; // 읽기 시간 기록
    return NULL;
}

// 쓰기 스레드 함수
void* writer(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    rwlock_acquire_write_lock(&rwlock); // 쓰기 잠금 획득
    printf("Writer acquired the write lock\n");

    // 공유 배열을 임의의 값으로 초기화
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        shared_array[i] = rand() % 100;
    }

    rwlock_release_write_lock(&rwlock); // 쓰기 잠금 해제
    printf("Writer released the write lock\n");

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    write_time = micros; // 쓰기 시간 기록
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writer_thread;
    struct timeval start, end;

    rwlock_init(&rwlock); // RWLock 초기화
    srand(time(NULL));

    // 시작 시간 측정
    gettimeofday(&start, NULL);

    // 쓰기 스레드 생성
    pthread_create(&writer_thread, NULL, writer, NULL);

    // 5개의 읽기 스레드 생성
    for (int i = 0; i < NUM_READERS; i++)
    {
        pthread_create(&readers[i], NULL, reader, (void*)&i); // 스레드 생성 및 매개변수 전달
    }

    // 쓰기 스레드 종료 대기
    pthread_join(writer_thread, NULL);

    // 읽기 스레드 종료 대기
    for (int i = 0; i < NUM_READERS; i++)
    {
        pthread_join(readers[i], NULL);
    }

    // 종료 시간 측정
    gettimeofday(&end, NULL);

    // 실행 시간 계산 및 출력
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    printf("실행 시간: %ld 초 %ld 마이크로초\n", seconds, micros);

    // 개별 스레드 시간 계산 및 출력
    long total_read_time = 0;
    for (int i = 0; i < NUM_READERS; i++)
    {
        total_read_time += read_times[i];
    }
    printf("Total read time: %ld 마이크로초\n", total_read_time);
    printf("Average read time: %ld 마이크로초\n", total_read_time / NUM_READERS);

    return 0;
}
