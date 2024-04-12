#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int sockfd, cSockfd;
    char buf[BUFSIZE];
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    // 소켓 재사용을 위한 옵션 설정
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    // 소켓과 서버 주소 바인딩
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        return 1;
    }

    // 연결 요청 대기열 설정
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        return 1;
    }

    // 클라이언트 연결 수락
    len = sizeof(cliaddr);
    if ((cSockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len)) == -1) {
        perror("accept");
        return 1;
    }

    // 클라이언트로부터 데이터 읽기
    ssize_t bytes_read;
    while ((bytes_read = read(cSockfd, buf, sizeof(buf))) > 0) {
        buf[bytes_read] = '\0'; // 문자열 종료를 위해 널 문자 추가
        printf("%s", buf); // 받은 데이터 출력
    }
    if (bytes_read == -1) {
        perror("read");
        return 1;
    }

    // 소켓 닫기
    close(sockfd);
    close(cSockfd);
    return 0;
}