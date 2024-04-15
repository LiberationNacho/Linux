#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFSIZE 1024

// 클라이언트 정보를 저장하는 구조체
typedef struct {
    int sockfd; // 클라이언트 소켓 디스크립터
    struct sockaddr_in addr; // 클라이언트 주소 정보
} ClientInfo;

ClientInfo clients[MAX_CLIENTS]; // 최대 클라이언트 수만큼의 배열

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 스레드 동기화를 위한 뮤텍스

/*
`void *`은 C 언어에서 일반적으로 사용되는 포인터 형식입니다.
여기서 `handle_client` 함수는 스레드의 시작 루틴으로 사용됩니다.
스레드의 시작 루틴은 `void *` 형식의 매개변수를 받아야 합니다.
그러나 실제로는 이 매개변수를 사용하지 않을 수도 있습니다. 때문에 `handle_client` 함수의 매개변수는 `void *` 타입으로 지정됩니다. 

이러한 형태는 일반적으로 스레드 함수에서 특정 데이터나 구조체를 전달할 필요가 있는 경우에 사용됩니다.
예를 들어, 여기서는 클라이언트 소켓 디스크립터를 스레드로 전달하여 해당 클라이언트와의 통신을 처리합니다.
이를 위해 `void *` 형식의 포인터를 사용하여 클라이언트 소켓 디스크립터를 전달하고, 함수 내에서 이를 다시 적절한 형식으로 형변환하여 사용합니다.
*/
// 클라이언트와의 통신을 담당하는 함수
void *handle_client(void *arg) {
    int cSockfd = *((int *)arg);
    char buf[BUFSIZE];

    // 클라이언트가 보낸 메시지를 받아서 다른 클라이언트에게 전달하고 화면에 출력
    while (1) {
        memset(buf, 0, sizeof(buf));
        int bytes_read = read(cSockfd, buf, sizeof(buf));
        if (bytes_read <= 0) {
            // 읽기가 실패하면 클라이언트 연결 종료
            close(cSockfd);
            pthread_exit(NULL);
        }

        // 메시지를 받은 클라이언트의 주소 정보를 찾음
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        getpeername(cSockfd, (struct sockaddr *)&sender_addr, &sender_len);

        // 받은 메시지를 다른 클라이언트에게 전송하고 화면에 출력
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].sockfd != -1 && clients[i].sockfd != cSockfd) {
                write(clients[i].sockfd, buf, strlen(buf));
            }
        }
        printf("Received from %s:%d: %s", inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port), buf);
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
    struct sockaddr_in servaddr;

    // 소켓 생성
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

	/*
	    bind(): 소켓에 주소를 할당하는 역할
	    소켓을 로컬 주소와 포트에 바인딩하여 해당 주소와 포트로 들어오는 연결을 수락하거나 데이터를 받을 수 있도록 준비한다.
	    sockfd: 바인딩할 소켓의 파일 디스크립터입니다. socket() 함수로 생성한 소켓의 파일 디스크립터를 전달합니다.
	    addr: 바인딩할 주소 정보를 담고 있는 struct sockaddr 구조체의 포인터입니다. 보통 struct sockaddr_in 구조체를 사용합니다.
	    addrlen: addr 구조체의 크기를 나타내는 값입니다.
	*/
    // 소켓에 주소 연결
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    /*
	   listen(): 소켓이 들어오는 연결을 수신할 수 있도록 설정하는 역할
	   sockfd: 연결을 대기할 소켓의 파일 디스크립터입니다. socket() 함수로 생성한 소켓의 파일 디스크립터를 전달합니다.
	   backlog: 연결 요청 대기 큐의 최대 길이를 지정합니다. 이는 동시에 처리 가능한 최대 연결 요청 수를 결정합니다.
	*/
    // 연결 요청 대기
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        close(sockfd);
        return 1;
    }

    // 클라이언트와의 연결을 처리할 스레드 생성
    while (1) {
        struct sockaddr_in cliaddr;

        // typedef int socklen_t;
        //소켓 API함수에서 주소 구조체의 크기를 전달하는데 사용되는 데이터 타입
        socklen_t len = sizeof(cliaddr);
        int cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (cSockfd == -1) {
            perror("accept");
            return 1;
        }

        // 클라이언트 배열에 추가
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].sockfd == -1) {
                clients[i].sockfd = cSockfd;
                clients[i].addr = cliaddr;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        // 최대 연결 수를 초과하는 경우 연결 종료
        if (client_count > MAX_CLIENTS) {
            printf("Maximum number of clients reached. Connection rejected.\n");
            close(cSockfd); // 소켓 닫기
            continue;
        }

        /*
        thread: 새로 생성된 스레드의 식별자를 저장할 변수의 포인터입니다.
        attr: 스레드의 속성을 지정하는데 사용되는 옵션입니다. 보통 NULL로 설정하여 기본 속성을 사용합니다.
        start_routine: 새로운 스레드가 실행할 함수의 포인터입니다.
        arg: start_routine 함수에 전달될 인자입니다.
        */
        // 새로운 스레드를 생성하여 클라이언트와 통신
        //  POSIX스레드를 나타내는 데이터 형식, 각 스레드의 고유한 식별자
        // fork()함수의 pid와 비슷한 듯
        pthread_t tid;
        if(pthread_create(&tid, NULL, handle_client, &cSockfd) != 0) {
            perror("pthread_create");
            return 1;
        }

	    // 현재 연결된 클라이언트 수 출력
        printf("Client connected. Current client count: %d\n", client_count);
    }

    // 소켓 닫기
    close(sockfd);
    return 0;
}
