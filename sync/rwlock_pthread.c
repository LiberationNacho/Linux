#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <math.h>

#define ARRAY_SIZE 100
int shared_array[ARRAY_SIZE];

typedef struct {
    pthread_rwlock_t lock;
} RWLock;

void rwlock_init(RWLock *rwlock) {
    pthread_rwlock_init(&rwlock->lock, NULL);
}

void rwlock_acquire_read_lock(RWLock *rwlock) {
    pthread_rwlock_rdlock(&rwlock->lock);
}

void rwlock_release_read_lock(RWLock *rwlock) {
    pthread_rwlock_unlock(&rwlock->lock);
}

void rwlock_acquire_write_lock(RWLock *rwlock) {
    pthread_rwlock_wrlock(&rwlock->lock);
}

void rwlock_release_write_lock(RWLock *rwlock) {
    pthread_rwlock_unlock(&rwlock->lock);
}

int calculate_sum(int* array, int start_index, int end_index) {
    int sum = 0;
    for (int i = start_index; i < end_index; i++) {
        sum += array[i];
    }
    return sum;
}

int find_max(int* array, int start_index, int end_index) {
    int max = INT_MIN;
    for (int i = start_index; i < end_index; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

double calculate_average(int* array, int start_index, int end_index) {
    int sum = calculate_sum(array, start_index, end_index);
    return (double)sum / (end_index - start_index);
}

double calculate_variance(int* array, int start_index, int end_index) {
    double average = calculate_average(array, start_index, end_index);
    double variance = 0.0;
    for (int i = start_index; i < end_index; i++) {
        variance += (array[i] - average) * (array[i] - average);
    }
    return variance / (end_index - start_index);
}

double calculate_stddev(double variance) {
    return sqrt(variance);
}

RWLock rwlock;
long read_times[5];
long write_time;

void* reader(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int index = *(int*)arg;
    int chunk_size = ARRAY_SIZE / 5;
    int start_index = index * chunk_size;
    int end_index = start_index + chunk_size;

    rwlock_acquire_read_lock(&rwlock);
    printf("Reader %d acquired the read lock\n", index);

    switch (index) {
        case 0:
            // 합계 계산
            int sum = calculate_sum(shared_array, start_index, end_index);
            printf("Reader %d calculated sum: %d\n", index, sum);
            break;
        case 1:
            // 최대값 계산
            int max = find_max(shared_array, start_index, end_index);
            printf("Reader %d found max: %d\n", index, max);
            break;
        case 2:
            // 평균 계산
            double average = calculate_average(shared_array, start_index, end_index);
            printf("Reader %d calculated average: %.2f\n", index, average);
            break;
        case 3:
            // 분산 계산
            double variance = calculate_variance(shared_array, start_index, end_index);
            printf("Reader %d calculated variance: %.2f\n", index, variance);
            break;
        case 4:
            // 표준편차 계산
            double stddev = calculate_stddev(variance);
            printf("Reader %d calculated standard deviation: %.2f\n", index, stddev);
            break;
        default:
            break;
    }

    rwlock_release_read_lock(&rwlock);
    printf("Reader %d released the read lock\n", index);

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    read_times[index] = micros;
    return NULL;
}

void* writer(void* arg) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    rwlock_acquire_write_lock(&rwlock);
    printf("Writer acquired the write lock\n");

    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared_array[i] = rand() % 10000;
    }

    rwlock_release_write_lock(&rwlock);
    printf("Writer released the write lock\n");

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    write_time = micros;
    return NULL;
}

int main() {
    pthread_t readers[5], writer_thread;
    struct timeval start, end;

    rwlock_init(&rwlock);
    srand(time(NULL));

    gettimeofday(&start, NULL);

    for (int i = 0; i < 5; i++) {
        int *thread_args = malloc(sizeof(int));
        *thread_args = i;
        pthread_create(&readers[i], NULL, reader, (void*)thread_args);
    }

    pthread_create(&writer_thread, NULL, writer, NULL);

    for (int i = 0; i < 5; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_join(writer_thread, NULL);

    gettimeofday(&end, NULL);

    long seconds = end.tv_sec - start.tv_sec;
    long micros = ((seconds * 1000000) + end.tv_usec) - start.tv_usec;
    printf("Execution time: %ld seconds and %ld microseconds\n", seconds, micros);

    long total_read_time = 0;
    for (int i = 0; i < 5; i++) {
        total_read_time += read_times[i];
    }
    printf("Total read time: %ld microseconds\n", total_read_time);
    printf("Total write time: %ld microseconds\n", write_time);
    printf("Average read time: %ld microseconds\n", total_read_time / 5);

    return 0;
}
