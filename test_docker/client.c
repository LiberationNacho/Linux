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
volatile sig_atomic_t flag = 0; // 서버 연결 끊김 플래그

int main() {
    struct sockaddr_in server_addr;
    pthread_t receive_thread;

    // SIGINT(CTRL+C) 시그널 처리
    signal(SIGINT, signal_handler);

    // 클라이언트 소켓 생성
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Could not create socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 서버에 연결
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(client_socket);
        return 1;
    }

    // 클라이언트 이름 입력 및 서버에 전송
    printf("Enter your name: ");
    fgets(name, NAME_SIZE, stdin);
    name[strcspn(name, "\n")] = 0; // 개행 문자 제거
    send(client_socket, name, NAME_SIZE, 0);

    // 메시지 수신 처리 스레드 생성
    if (pthread_create(&receive_thread, NULL, receive_handler, (void*)&client_socket) != 0) {
        perror("Could not create thread");
        close(client_socket);
        return 1;
    }

    char message[BUFFER_SIZE];
    while (1) {
        if (flag) break; // 서버와의 연결이 끊어지면 루프 종료

        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // 개행 문자 제거

        if (strlen(message) > 0) {
            send(client_socket, message, strlen(message), 0); // 메시지 전송
        }
    }

    // 자원 정리
    close(client_socket);
    pthread_join(receive_thread, NULL);
    return 0;
}

// 메시지 수신 처리 함수
void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;
    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s\n", buffer); // 수신한 메시지 출력
    }

    if (read_size == 0 || read_size == -1) {
        printf("Disconnected from server.\n");
        flag = 1; // 연결 끊김 플래그 설정
    }

    return NULL;
}

// 클라이언트 종료 처리 함수
void signal_handler(int sig) {
    printf("Disconnecting from server...\n");
    close(client_socket);
    exit(0);
}

