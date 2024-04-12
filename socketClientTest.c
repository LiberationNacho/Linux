#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
        if (write(sockfd, buf, strlen(buf)) == -1) {
            perror("write");
            break;
        }
    }

    // 소켓 닫기
    close(sockfd);
    return 0;
}