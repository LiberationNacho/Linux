#include <stdio.h>
#include <string.h>
// unistd.h는 원래 리눅스용 헤더이지만, VS에는 io.h
// #include <unistd.h>
// #include <io.h>
#include <stdlib.h>
// 동일한 이슈
// #include <sys/socket.h>
#include <winsock2.h>
// #include <arpa/inet.h>
#include <ws2tcpip.h>

#define BUFSIZE 1024

/*
int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <server_ip>\n", argv[0]);
        return 1;
    }

    int sockfd;
    char buf[BUFSIZE];
    struct sockaddr_in servaddr;

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    if (inet_pton(AF_INET, argv[2], &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("connect");
        return 1;
    }

    // 메시지 전송 루프
    while (1) {
        memset(buf, 0, sizeof(buf));
        printf("Input Message: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            perror("fgets");
            break;
        }
        if (write(sockfd, buf, strlen(buf)) == -1) {
            perror("write");
            break;
        }
    }

    // 소켓 닫기
    close(sockfd);
    return 0;
}*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h> // winsock.h 대신 winsock2.h 사용
#include <ws2tcpip.h>

#define BUFSIZE 1024

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <server_ip>\n", argv[0]);
        return 1;
    }

    int sockfd;
    char buf[BUFSIZE];
    struct sockaddr_in servaddr;

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    if (inet_pton(AF_INET, argv[2], &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("connect");
        return 1;
    }

    // 메시지 전송 루프
    while (1) {
        memset(buf, 0, sizeof(buf));
        printf("Input Message: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            perror("fgets");
            break;
        }
        if (send(sockfd, buf, strlen(buf), 0) == -1) { // write 대신 send 사용
            perror("send");
            break;
        }
    }

    // 소켓 닫기
    closesocket(sockfd); // close 대신 closesocket 사용
    return 0;
}
