#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define NUM_THREADS 10
#define ARRAY_SIZE 10000000
#define ITERATIONS_PER_THREAD 100

int array[ARRAY_SIZE];

// 싱글 스레드로 실행되는 함수
void *single_thread_function(void *arg) {
    long long sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ITERATIONS_PER_THREAD; j++) {
            sum += array[i];
        }
    }

    printf("싱글 스레드 결과: %lld\n", sum);

    return NULL;
}

// 멀티 스레드로 실행되는 함수
void *mult_thread_function(void *arg) {
    long long sum = 0;
    int thread_id = *((int*)arg);
    int chunk_size = ARRAY_SIZE / NUM_THREADS;
    int start = thread_id * chunk_size;
    int end = (thread_id == NUM_THREADS - 1) ? ARRAY_SIZE : start + chunk_size;

    for (int i = start; i < end; i++) {
        for (int j = 0; j < ITERATIONS_PER_THREAD; j++) {
            sum += array[i];
        }
    }

    return (void *)(intptr_t) sum;
}

// 싱글 스레드 실행 시간을 측정하는 함수
struct timeval do_single_thread() {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    single_thread_function(NULL);

    gettimeofday(&end, NULL);
    timersub(&end, &start, &end);
    return end;
}

// 멀티 스레드 실행 시간을 측정하는 함수
struct timeval do_multi_thread() {
    struct timeval start, end;
    void *return_value;
    long long sum = 0;
    gettimeofday(&start, NULL);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, mult_thread_function, &thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], &return_value);
        long long value = (intptr_t)return_value;
        sum += value;
    }

    printf("멀티 스레드 결과: %lld\n", sum);
    gettimeofday(&end, NULL);
    timersub(&end, &start, &end);
    return end;
}

// 두 실행 시간의 차이를 출력하는 함수
void print_diff(struct timeval single_time, struct timeval multi_time) {
    long int single_usec = single_time.tv_sec * 1000000 + single_time.tv_usec;
    long int multi_usec = multi_time.tv_sec * 1000000 + multi_time.tv_usec;
    printf("싱글 스레드 실행 시간: %ld 마이크로초\n", single_usec);
    printf("멀티 스레드 실행 시간: %ld 마이크로초\n", multi_usec);
    printf("차이: %ld 마이크로초\n", single_usec - multi_usec);
    printf("싱글 스레드 실행 시간은 멀티 스레드 실행 시간의 %.2f배입니다.\n", (float)single_usec / multi_usec);
}

int main(int argc, char* argv[]) {
    // 난수 시드 초기화
    srand(time(NULL));

    // 배열 초기화
    for (int i = 0; i < ARRAY_SIZE; i++) array[i] = rand() % 100;

    struct timeval single_thread_processing_time = do_single_thread();
    struct timeval multi_thread_processing_time = do_multi_thread();

    print_diff(single_thread_processing_time, multi_thread_processing_time);

    return 0;
}
