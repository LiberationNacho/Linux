#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024

int sockfd;

// 서버로부터 메시지를 받는 함수
void *receive_message(void *arg) {
    char buf[BUFSIZE];
    while (1) {
        // 서버가 보낸 응답 받기
        memset(buf, 0, sizeof(buf));
        if (read(sockfd, buf, sizeof(buf)) == -1) {
            perror("read");
            pthread_exit(NULL);
        }
        printf("Server Response: %s", buf);
        // 만약 서버로부터 받은 메시지가 "exit"인 경우 프로그램 종료
        if (strcmp(buf, "exit\n") == 0) {
            printf("Exiting...\n");
            close(sockfd);
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
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

    // 서버로부터 메시지를 받는 스레드 생성
    pthread_t tid;
    if (pthread_create(&tid, NULL, receive_message, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    // 메시지 주고받기
    char buf[BUFSIZE];
    while (1) {
        printf("Input Message : ");
        fgets(buf, sizeof(buf), stdin);

        // 클라이언트가 "exit"를 입력한 경우 프로그램 종료
        if (strcmp(buf, "exit\n") == 0) {
            printf("Exiting...\n");
            close(sockfd);
            exit(0);
        }

        // 클라이언트가 서버로 메시지 전송
        write(sockfd, buf, strlen(buf));
    }

    // 연결 종료
    close(sockfd);
    return 0;
}