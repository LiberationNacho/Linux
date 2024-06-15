#include <stdio.h>      // 표준 입력/출력 함수
#include <stdlib.h>     // 표준 라이브러리 함수
#include <string.h>     // 문자열 관련 함수
#include <unistd.h>     // POSIX 운영 체제 API
#include <arpa/inet.h>  // 인터넷 주소 변환 함수
#include <pthread.h>    // POSIX 스레드 관련 함수
#include <signal.h>     // 시그널 처리 함수

#define PORT 8080       // 서버 포트 번호
#define BUFFER_SIZE 2048// 버퍼 크기
#define NAME_SIZE 32    // 클라이언트 이름 최대 길이

int client_socket;      // 클라이언트 소켓
char name[NAME_SIZE];   // 클라이언트 이름

// 함수 선언
void *receive_handler(void *socket_desc);   // 수신 처리 함수
void *send_handler(void *socket_desc);      // 송신 처리 함수
void signal_handler(int sig);               // 시그널 처리 함수

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;     // 서버 주소 구조체
    pthread_t receive_thread, send_thread;   // 수신 및 송신 스레드

    signal(SIGINT, signal_handler);     // SIGINT(CTRL+C) 시그널 핸들러 등록

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);   // TCP 소켓 생성
    if (client_socket == -1)
    {
        perror("Socket creation failed");   // 소켓 생성 실패 시 오류 메시지 출력
        return 1;
    }

    server_addr.sin_family = AF_INET;   // IPv4 주소 체계 설정
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {   // 서버 IP 주소 변환
        perror("Invalid address/ Address not supported");   // 유효하지 않은 주소 처리
        return 1;
    }
    server_addr.sin_port = htons(PORT); // 포트 설정

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Connection failed");    // 연결 실패 시 오류 메시지 출력
        close(client_socket);   // 소켓 닫기
        return 1;
    }

    printf("Client start\n");
    
    printf("Enter your name: ");
    fgets(name, NAME_SIZE, stdin);  // 사용자 이름 입력
    name[strcspn(name, "\n")] = 0;  // fgets로 입력받은 개행 문자 제거
    send(client_socket, name, strlen(name), 0);   // 서버로 이름 전송

    // 수신 및 송신 스레드 생성
    if (pthread_create(&receive_thread, NULL, receive_handler, (void*)&client_socket) != 0)
    {
        perror("Receive thread creation failed");   // 수신 스레드 생성 실패 시 오류 메시지 출력
        close(client_socket);   // 소켓 닫기
        return 1;
    }

    if (pthread_create(&send_thread, NULL, send_handler, (void*)&client_socket) != 0)
    {
        perror("Send thread creation failed");  // 송신 스레드 생성 실패 시 오류 메시지 출력
        close(client_socket);   // 소켓 닫기
        return 1;
    }

    pthread_join(receive_thread, NULL);   // 수신 스레드 종료 대기
    pthread_join(send_thread, NULL);      // 송신 스레드 종료 대기

    close(client_socket);   // 클라이언트 소켓 닫기
    return 0;
}

// 수신 처리 함수
void *receive_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;     // 소켓 번호 가져오기
    char buffer[BUFFER_SIZE];           // 버퍼 선언
    int read_size;

    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
    {   // 데이터 수신
        buffer[read_size] = '\0';   // 버퍼 끝에 널 문자 추가하여 문자열 완성
        printf("%s", buffer);       // 수신한 메시지 출력
    }
    return NULL;
}

// 송신 처리 함수
void *send_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;     // 소켓 번호 가져오기
    char message[BUFFER_SIZE];         // 메시지 버퍼 선언

    while (1)
    {
        fgets(message, BUFFER_SIZE, stdin); // 사용자로부터 메시지 입력 받기
        send(sock, message, strlen(message), 0);   // 서버로 메시지 전송
    }
    return NULL;
}

// 시그널 처리 함수
void signal_handler(int sig)
{
    char c;
    printf("종료하시겠습니까? (y/n)\n");
    scanf(" %c", &c);

    if (c == 'y' || c == 'Y')
    {
        printf("Shutting down client...\n");    // 종료 메시지 출력
        close(client_socket);   // 클라이언트 소켓 닫기
        exit(0);    // 프로그램 종료
    }
    else
    {
        return;     // 종료하지 않음
    }
}
