#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 2048
#define NAME_SIZE 32

int client_socket; // 클라이언트 소켓
void *receive_handler(void *socket_desc); // 메시지 수신 처리 함수
void signal_handler(int sig); // 시그널 핸들러 함수
char name[NAME_SIZE]; // 클라이언트 이름

int main() {
    struct sockaddr_in server_addr;
    pthread_t receive_thread;

    // SIGINT(CTRL+C) 시그널 처리
    signal(SIGINT, signal_handler);

    // 클라이언트 소켓 생성
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 서버에 연결
    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 클라이언트 이름 입력 및 서버에 전송
    printf("Enter your name: ");
    fgets(name, NAME_SIZE, stdin);
    name[strcspn(name, "\n")] = 0; // 개행 문자 제거
    send(client_socket, name, NAME_SIZE, 0);

    // 메시지 수신 처리 스레드 생성
    pthread_create(&receive_thread, NULL, receive_handler, (void*)&client_socket);

    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        send(client_socket, message, strlen(message), 0); // 메시지 전송
    }

    close(client_socket);
    return 0;
}

// 메시지 수신 처리 함수
void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;
    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer); // 수신한 메시지 출력
    }
    return 0;
}

// 클라이언트 종료 처리 함수
void signal_handler(int sig) {
    printf("Disconnecting from server...\n");
    close(client_socket);
    exit(0);
}

