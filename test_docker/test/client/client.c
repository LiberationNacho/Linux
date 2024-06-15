#include <stdio.h>      // 표준 입출력 함수 사용을 위한 헤더 파일
#include <stdlib.h>     // 표준 라이브러리 함수 사용을 위한 헤더 파일
#include <string.h>     // 문자열 처리 함수 사용을 위한 헤더 파일
#include <unistd.h>     // POSIX 운영 체제 API 접근을 위한 헤더 파일
#include <arpa/inet.h>  // 인터넷 프로토콜 관련 함수 사용을 위한 헤더 파일
#include <pthread.h>    // POSIX 스레드 사용을 위한 헤더 파일
#include <signal.h>     // 시그널 처리 함수 사용을 위한 헤더 파일

#define PORT 8080       // 서버가 사용할 포트 번호
#define BUFFER_SIZE 2048 // 버퍼 크기
#define NAME_SIZE 32    // 클라이언트 이름 최대 길이

int client_socket; // 클라이언트 소켓
char name[NAME_SIZE]; // 클라이언트 이름

// 함수 선언
void *receive_handler(void *socket_desc); // 메시지 수신 처리 함수
void signal_handler(int sig); // 시그널 핸들러 함수

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr; // 서버 주소 구조체
    pthread_t receive_thread; // 메시지 수신 처리 스레드

    // SIGINT(CTRL+C) 시그널 처리
    signal(SIGINT, signal_handler);

    // 인자가 충분하지 않을 경우 사용법 출력
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    // 클라이언트 소켓 생성
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }
    server_addr.sin_port = htons(PORT);

    // 서버에 연결
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        return 1;
    }

    printf("Client start\n");
    
    // 클라이언트 이름 입력 및 서버에 전송
    printf("Enter your name: ");
    fgets(name, NAME_SIZE, stdin); // 사용자로부터 이름 입력받기
    name[strcspn(name, "\n")] = 0; // 개행 문자 제거
    send(client_socket, name, strlen(name), 0); // 서버에 이름 전송

    // 메시지 수신 처리 스레드 생성
    if (pthread_create(&receive_thread, NULL, receive_handler, (void*)&client_socket) != 0) {
        perror("Thread creation failed");
        close(client_socket);
        return 1;
    }

    // 메시지 입력 및 전송
    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin); // 사용자로부터 메시지 입력받기
        send(client_socket, message, strlen(message), 0); // 서버로 메시지 전송
    }

    close(client_socket); // 클라이언트 소켓 닫기
    return 0;
}

// 메시지 수신 처리 함수
void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    // 서버로부터 메시지 수신 및 출력
    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0'; // 문자열 종료
        printf("%s", buffer); // 수신한 메시지 출력
    }
    return NULL;
}

// 시그널 처리 함수
void signal_handler(int sig) {
    char c;
    printf("종료하시겠습니까? (y/n)\n");
    scanf(" %c", &c); // 공백을 포함하여 한 문자를 입력받음

    if (c == 'y' || c == 'Y') {
        printf("Shutting down client...\n");
        exit(0); // 프로그램을 종료
    } else {
        // 다른 입력은 아무런 동작도 하지 않고 함수를 종료
        return;
    }
}