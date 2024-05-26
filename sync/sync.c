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
void print_rankings();

void initialize_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        rankings[i] = rand() % 100;
    }
}

void print_rankings() {
    for (int i = 0; i < NUM_RANKINGS; i++) {
        printf("%d ", rankings[i]);
    }
    printf("\n");
}

void* read_rankings_rwlock(void* arg) {
    pthread_rwlock_rdlock(&rwlock);
    printf("RWLock Read: ");
    print_rankings();
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void* write_rankings_rwlock(void* arg) {
    pthread_rwlock_wrlock(&rwlock);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    printf("RWLock Write: Updated index %d with value %d\n", index, value);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void* read_rankings_mutex(void* arg) {
    pthread_mutex_lock(&mutex);
    printf("Mutex Read: ");
    print_rankings();
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* write_rankings_mutex(void* arg) {
    pthread_mutex_lock(&mutex);
    int index = rand() % NUM_RANKINGS;
    int value = rand() % 100;
    rankings[index] = value;
    printf("Mutex Write: Updated index %d with value %d\n", index, value);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    srand(time(NULL));
    initialize_rankings();

    pthread_rwlock_init(&rwlock, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[20];

    // RWLock 사용
    printf("Using RWLock:\n");
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, read_rankings_rwlock, NULL);
    }
    for (int i = 5; i < 10; i++) {
        pthread_create(&threads[i], NULL, write_rankings_rwlock, NULL);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    // Mutex 사용
    printf("\nUsing Mutex:\n");
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, read_rankings_mutex, NULL);
    }
    for (int i = 5; i < 10; i++) {
        pthread_create(&threads[i], NULL, write_rankings_mutex, NULL);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}
