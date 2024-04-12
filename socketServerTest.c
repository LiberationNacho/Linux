/*
#include <stdio.h>
// unistd.h는 원래 리눅스용 헤더이지만, VS에는 io.h
// #include <unistd.h>
// #include <io.h>
#include <stdlib.h>
#include <string.h>
// 동일한 이슈
// #include <sys/socket.h>
#include <winsock.h>
// #include <arpa/inet.h>
#include <ws2tcpip.h>

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
}*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // winsock.h 대신 winsock2.h 사용
#include <ws2tcpip.h>

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

    // 윈속 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup");
        return 1;
    }

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) { // -1 대신 INVALID_SOCKET 사용
        perror("socket");
        return 1;
    }

    // 소켓 재사용을 위한 옵션 설정
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) == SOCKET_ERROR) { // -1 대신 SOCKET_ERROR 사용
        perror("setsockopt");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    // 소켓과 서버 주소 바인딩
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) { // -1 대신 SOCKET_ERROR 사용
        perror("bind");
        return 1;
    }

    // 연결 요청 대기열 설정
    if (listen(sockfd, 5) == SOCKET_ERROR) { // -1 대신 SOCKET_ERROR 사용
        perror("listen");
        return 1;
    }

    // 클라이언트 연결 수락
    len = sizeof(cliaddr);
    if ((cSockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len)) == INVALID_SOCKET) { // -1 대신 INVALID_SOCKET 사용
        perror("accept");
        return 1;
    }

    // 클라이언트로부터 데이터 읽기
    size_t bytes_read;
    while ((bytes_read = recv(cSockfd, buf, sizeof(buf), 0)) > 0) { // read 대신 recv 사용
        buf[bytes_read] = '\0'; // 문자열 종료를 위해 널 문자 추가
        printf("%s", buf); // 받은 데이터 출력
    }
    if (bytes_read == SOCKET_ERROR) { // -1 대신 SOCKET_ERROR 사용
        perror("recv");
        return 1;
    }

    // 소켓 닫기
    closesocket(sockfd); // close 대신 closesocket 사용
    closesocket(cSockfd); // close 대신 closesocket 사용

    // 윈속 종료
    WSACleanup();

    return 0;
}
