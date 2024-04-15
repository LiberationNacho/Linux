#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024

// 출력을 동기화하기 위한 mutex 선언
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 서버 응답을 처리하는 스레드 함수
void *receive_response(void *arg) {
    int sockfd = *((int *)arg);
    char buf[BUFSIZE];

    while (1) {
        // 서버가 보낸 응답 받기
        memset(buf, 0, sizeof(buf));
        if (read(sockfd, buf, sizeof(buf)) == -1) {
            perror("read");
            pthread_exit(NULL);
        }

        // 출력 동기화
        pthread_mutex_lock(&mutex);
        printf("Server Response: %s", buf);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in servaddr;

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("connect");
        return 1;
    }

    // 응답을 받는 스레드 생성
    pthread_t tid;
    if (pthread_create(&tid, NULL, receive_response, &sockfd) != 0) {
        perror("pthread_create");
        return 1;
    }

    // 메시지 주고받기
    char buf[BUFSIZE];
    while (1) {
        printf("Input Message : ");
        fgets(buf, sizeof(buf), stdin);

        // 클라이언트가 서버로 메시지 전송
        write(sockfd, buf, strlen(buf));
    }

    // 연결 종료
    close(sockfd);
    return 0;
}
