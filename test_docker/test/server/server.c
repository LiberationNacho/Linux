#include <stdio.h>      // 표준 입출력 함수 사용을 위한 헤더 파일
#include <stdlib.h>     // 표준 라이브러리 함수 사용을 위한 헤더 파일
#include <string.h>     // 문자열 처리 함수 사용을 위한 헤더 파일
#include <unistd.h>     // POSIX 운영 체제 API 접근을 위한 헤더 파일
#include <pthread.h>    // POSIX 스레드 사용을 위한 헤더 파일
#include <arpa/inet.h>  // 인터넷 프로토콜 관련 함수 사용을 위한 헤더 파일
#include <signal.h>     // 시그널 처리 함수 사용을 위한 헤더 파일

#define PORT 8080       // 서버가 사용할 포트 번호
#define MAX_CLIENTS 100 // 최대 클라이언트 수
#define BUFFER_SIZE 2048 // 버퍼 크기
#define NAME_SIZE 32    // 클라이언트 이름 최대 길이
#define THREAD_POOL_SIZE 10 // 스레드 풀 크기

// 클라이언트 정보를 담을 구조체 정의
typedef struct {
    int socket;           // 클라이언트 소켓
    char name[NAME_SIZE]; // 클라이언트 이름
} client_t;

// 전역 변수 선언
client_t *clients[MAX_CLIENTS]; // 클라이언트 리스트
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // 클라이언트 리스트 보호용 뮤텍스

pthread_t thread_pool[THREAD_POOL_SIZE]; // 스레드 풀
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER; // 조건 변수
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER; // 조건 변수 보호용 뮤텍스
client_t *client_queue[MAX_CLIENTS]; // 클라이언트 큐
int queue_size = 0; // 큐 크기

// 함수 선언
void *handle_client(void *arg); // 클라이언트 처리 함수 (스레드 함수)
void send_message(char *message, client_t *exclude_client); // 메시지 전송 함수
void signal_handler(int sig); // 시그널 핸들러 함수
void *thread_function(void *arg); // 스레드 풀에서 사용할 함수
void enqueue_client(client_t *client); // 클라이언트를 큐에 추가
client_t *dequeue_client(); // 큐에서 클라이언트를 꺼내기

int main() {
    int server_socket, client_socket; // 서버 소켓과 클라이언트 소켓
    struct sockaddr_in server_addr, client_addr; // 서버와 클라이언트 주소 구조체
    socklen_t client_addr_len = sizeof(client_addr); // 클라이언트 주소 구조체 크기

    // SIGINT(CTRL+C) 시그널 처리
    signal(SIGINT, signal_handler);

    // 서버 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Could not create socket");
        return 1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 소켓 바인드
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    // 리스닝 설정
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    printf("Server start..\n");
    printf("Chat server started on port %d\n", PORT);

    // 스레드 풀 생성
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    // 클라이언트 연결 대기
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // 새로운 클라이언트 연결 처리(뮤텍스 락으로 보호)
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == NULL) {
                // 새로운 클라이언트 연결 처리
                client_t *new_client = (client_t*)malloc(sizeof(client_t));
                new_client->socket = client_socket;
                recv(client_socket, new_client->name, NAME_SIZE, 0); // 클라이언트 이름 수신
                clients[i] = new_client;
                enqueue_client(new_client); // 클라이언트를 큐에 추가
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        // 뮤텍스 언락
    }

    // 서버 소켓 닫기
    close(server_socket);
    return 0;
}

// 클라이언트를 큐에 추가하는 함수
void enqueue_client(client_t *client) {
    pthread_mutex_lock(&condition_mutex);
    client_queue[queue_size++] = client; // 큐에 클라이언트 추가
    pthread_cond_signal(&condition_var); // 조건 변수 시그널
    pthread_mutex_unlock(&condition_mutex);
}

// 큐에서 클라이언트를 꺼내는 함수
client_t *dequeue_client() {
    client_t *client = client_queue[0]; // 큐의 첫 번째 클라이언트
    for (int i = 1; i < queue_size; i++) {
        client_queue[i - 1] = client_queue[i]; // 큐를 앞으로 이동
    }
    queue_size--;
    return client;
}

// 스레드 풀에서 사용할 함수
void *thread_function(void *arg) {
    while (1) {
        pthread_mutex_lock(&condition_mutex);
        while (queue_size == 0) {
            pthread_cond_wait(&condition_var, &condition_mutex); // 큐가 비었으면 기다림
        }
        client_t *client = dequeue_client(); // 큐에서 클라이언트 꺼내기
        pthread_mutex_unlock(&condition_mutex);

        if (client != NULL) {
            handle_client(client); // 클라이언트 처리
        }
    }
    return NULL;
}

// 클라이언트 처리 함수 (스레드 함수)
void *handle_client(void *arg) {
    client_t *client = (client_t*)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    // 연결된 클라이언트의 참여 알림
    snprintf(buffer, sizeof(buffer), "--- 새로운 사용자 : %s이 들어왔습니다. ---\n", client->name);
    printf("%s", buffer); // 서버 콘솔에 출력
    send_message(buffer, client); // 모든 클라이언트에게 알림

    // 클라이언트로부터 메시지 수신 및 전송
    while ((read_size = recv(client->socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0'; // 문자열 종료

        // 메시지 작성
        size_t message_len = strlen(client->name) + 2 + strlen(buffer) + 1;
        char *message = (char*)malloc(message_len);
        if (message != NULL) {
            snprintf(message, message_len, "%s: %s", client->name, buffer);

            // 서버 콘솔에 클라이언트가 보낸 메시지 출력
            printf("%s", message);

            // 모든 클라이언트에게 메시지 전송
            send_message(message, client);
            free(message);
        }
    }

    // 클라이언트 연결 종료 처리
    close(client->socket);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == client) {
            clients[i] = NULL; // 클라이언트 리스트에서 제거
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    snprintf(buffer, sizeof(buffer), "------사용자 %s님이 나갔습니다.-----\n", client->name);
    printf("%s", buffer); // 서버 콘솔에 출력
    send_message(buffer, client); // 모든 클라이언트에게 알림

    free(client); // 클라이언트 메모리 해제
    return NULL;
}

// 메시지 전송 함수
void send_message(char *message, client_t *exclude_client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != NULL && clients[i] != exclude_client) {
            send(clients[i]->socket, message, strlen(message), 0); // 메시지 전송
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 서버 종료 처리 함수
void signal_handler(int sig) {
    char c;
    printf("종료하시겠습니까? (y/n)\n");
    scanf(" %c", &c); // 공백을 포함하여 한 문자를 입력받음

    if (c == 'y' || c == 'Y') {
        printf("Shutting down server...\n");
        exit(0); // 프로그램을 종료
    } else {
        // 다른 입력은 아무런 동작도 하지 않고 함수를 종료
        return;
    }
}