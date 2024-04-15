#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024
#define MAX_CLIENTS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[MAX_CLIENTS]; // 클라이언트 소켓을 저장하는 배열

// 클라이언트와의 통신을 담당하는 함수
void *handle_client(void *arg) {
    int cSockfd = *((int *)arg);
    char buf[BUFSIZE];

    while (1) {
        // 클라이언트로부터 메시지 수신
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_received = read(cSockfd, buf, sizeof(buf));
        if (bytes_received == -1) {
            perror("read");
            close(cSockfd); // 소켓 닫기
            pthread_exit(NULL);
        } else if (bytes_received == 0) {
            // 클라이언트가 연결을 끊은 경우
            printf("Client disconnected.\n");
            close(cSockfd); // 소켓 닫기
            pthread_exit(NULL);
        }

        // 받은 메시지를 다른 클라이언트에게 전파
        pthread_mutex_lock(&mutex);
        printf("Received from client: %s", buf);
        fflush(stdout);

        // 모든 클라이언트에게 메시지 전송
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] != -1 && clients[i] != cSockfd) {
                write(clients[i], buf, strlen(buf));
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd, cSockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    int clients[MAX_CLIENTS];
    pthread_t tid[MAX_CLIENTS];

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // 소켓과 주소를 바인딩
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        return 1;
    }

    // 클라이언트 연결 대기
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        return 1;
    }

    // 초기화: 클라이언트 배열에 -1로 초기화
    memset(clients, -1, sizeof(clients));

    // 클라이언트와의 연결을 처리할 스레드 생성
    int i = 0;
    while (1) {
        // 클라이언트 연결 대기 및 처리
        len = sizeof(cliaddr);
        cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (cSockfd == -1) {
            perror("accept");
            return 1;
        }
        
        // 새로운 클라이언트 소켓을 배열에 추가
        for (i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == -1) {
                clients[i] = cSockfd;
                break;
            }
        }

        // 새로운 스레드를 생성하여 클라이언트와 통신
        if (pthread_create(&tid[i], NULL, handle_client, &cSockfd) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    // 연결 종료
    close(sockfd);
    return 0;
}
