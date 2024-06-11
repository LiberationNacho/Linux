#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define NAME_SIZE 32

// 클라이언트 정보를 담을 구조체 정의
typedef struct {
    int socket;           // 클라이언트 소켓
    char name[NAME_SIZE]; // 클라이언트 이름
} client_t;

client_t *clients[MAX_CLIENTS]; // 클라이언트 리스트
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // 클라이언트 리스트 보호용 뮤텍스

void handle_client(client_t *client); // 클라이언트 처리 함수
void send_message(char *message, client_t *exclude_client); // 메시지 전송 함수
void signal_handler(int sig); // 시그널 핸들러 함수

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // SIGINT(CTRL+C) 시그널 처리
    signal(SIGINT, signal_handler);

    // 서버 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 소켓 바인드 및 리스닝 설정
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);

    printf("Chat server started on port %d\n", PORT);

    // 클라이언트 연결 대기
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == NULL) {
                // 새로운 클라이언트 연결 처리
                client_t *new_client = (client_t*)malloc(sizeof(client_t));
                new_client->socket = client_socket;
                recv(client_socket, new_client->name, NAME_SIZE, 0); // 클라이언트 이름 수신
                clients[i] = new_client;
                pthread_t tid;
                pthread_create(&tid, NULL, (void*)handle_client, (void*)new_client);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    return 0;
}

// 클라이언트 처리 함수
void handle_client(client_t *client) {
    char buffer[BUFFER_SIZE];
    int read_size;
    
    // 연결된 클라이언트의 참여 알림
    sprintf(buffer, "%s has joined\n", client->name);
    send_message(buffer, client);

    // 클라이언트로부터 메시지 수신 및 전송
    while ((read_size = recv(client->socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        char message[BUFFER_SIZE + NAME_SIZE];
        sprintf(message, "%s: %s", client->name, buffer); // 클라이언트 이름 포함한 메시지 작성
        send_message(message, client);
    }

    // 클라이언트 연결 종료 처리
    close(client->socket);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == client) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    sprintf(buffer, "%s has left\n", client->name);
    send_message(buffer, client);

    free(client);
}

// 메시지 전송 함수
void send_message(char *message, client_t *exclude_client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != NULL && clients[i] != exclude_client) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 서버 종료 처리 함수
void signal_handler(int sig) {
    printf("Shutting down server...\n");
    exit(0);
}

