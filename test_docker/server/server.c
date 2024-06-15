#include <stdio.h>      // 표준 입력/출력 함수
#include <stdlib.h>     // 표준 라이브러리 함수
#include <string.h>     // 문자열 관련 함수
#include <unistd.h>     // POSIX 운영 체제 API
#include <arpa/inet.h>  // 인터넷 주소 관련 정의
#include <pthread.h>    // POSIX 스레드 관련 함수
#include <signal.h>     // 시그널 처리 함수

#define PORT 8080       // 서버 포트 번호
#define MAX_CLIENTS 100 // 최대 클라이언트 수
#define BUFFER_SIZE 2048// 버퍼 크기
#define NAME_SIZE 32    // 클라이언트 이름 최대 길이
#define THREAD_POOL_SIZE 10 // 스레드 풀 크기

typedef struct {
    int socket;         // 클라이언트 소켓 번호
    char name[NAME_SIZE];// 클라이언트 이름
} client_t;             // 클라이언트 구조체

client_t *clients[MAX_CLIENTS]; // 클라이언트 배열
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;   // 클라이언트 배열 뮤텍스

pthread_t thread_pool[THREAD_POOL_SIZE];     // 스레드 풀
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER; // 조건 변수
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER; // 조건 변수 뮤텍스
client_t *client_queue[MAX_CLIENTS]; // 클라이언트 대기 큐
int queue_size = 0; // 대기 큐 크기

void *handle_client(void *arg);         // 클라이언트 처리 함수
void send_message(char *message, client_t *exclude_client); // 메시지 전송 함수
void signal_handler(int sig);           // 시그널 처리 함수
void *thread_function(void *arg);       // 스레드 실행 함수
void enqueue_client(client_t *client);  // 클라이언트 대기 큐에 추가 함수
client_t *dequeue_client();             // 클라이언트 대기 큐에서 가져오기 함수

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    signal(SIGINT, signal_handler); // SIGINT 시그널 처리 함수 등록

    // TCP 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("소켓 생성 실패");
        return 1;
    }

    server_addr.sin_family = AF_INET; // IPv4 주소 체계 설정
    server_addr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스에서 접속 허용
    server_addr.sin_port = htons(PORT); // 포트 설정

    // 소켓 바인드
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("바인드 실패");
        close(server_socket);
        return 1;
    }

    // 리스닝 설정
    if (listen(server_socket, 10) < 0)
    {
        perror("리스닝 실패");
        close(server_socket);
        return 1;
    }

    printf("서버 시작..\n");
    printf("채팅 서버가 포트 %d에서 시작되었습니다.\n", PORT);

    // 클라이언트 배열 초기화
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        clients[i] = NULL;
    }

    // 스레드 풀 생성
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    // 클라이언트 접속 처리(연결 대기)
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("접속 수락 실패");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; ++i)
        {
            if (clients[i] == NULL)
            {
                client_t *new_client = (client_t*)malloc(sizeof(client_t));
                new_client->socket = client_socket;
                recv(client_socket, new_client->name, NAME_SIZE, 0);
                clients[i] = new_client;
                enqueue_client(new_client);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_socket);
    return 0;
}

//클라이언트를 큐에 추가하는 함수
void enqueue_client(client_t *client)
{
    pthread_mutex_lock(&condition_mutex);
    client_queue[queue_size++] = client; // 큐에 클라이언트 추가
    pthread_cond_signal(&condition_var); // 대기 중인 스레드에 시그널 전송
    pthread_mutex_unlock(&condition_mutex);
}

//큐에서 클라이언트를 꺼내는 함수
client_t *dequeue_client()
{
    if (queue_size == 0)
    {
        return NULL;
    }
    client_t *client = client_queue[0]; // 첫 번째 클라이언트 가져오기
    for (int i = 1; i < queue_size; i++)
    {
        client_queue[i - 1] = client_queue[i]; // 나머지 클라이언트들 앞으로 이동
    }
    queue_size--; // 큐 크기 감소
    return client;
}

// 스레드 풀에서 사용할 함수
void *thread_function(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&condition_mutex);
        while (queue_size == 0)
        {
            pthread_cond_wait(&condition_var, &condition_mutex); // 대기 중인 클라이언트가 있을 때까지 대기
        }
        client_t *client = dequeue_client(); // 대기 중인 클라이언트 가져오기
        pthread_mutex_unlock(&condition_mutex);

        if (client != NULL)
        {
            handle_client(client); // 클라이언트 처리 함수 호출
        }
    }
    return NULL;
}

// 클라이언트 처리 함수 (스레드 함수)
void *handle_client(void *arg)
{
    client_t *client = (client_t*)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    // 다른 클라이언트에게 접속 알림 전송
    snprintf(buffer, sizeof(buffer), "--- 새로운 사용자 \"%s\" 접속. ---\n", client->name);
    printf("%s", buffer);
    send_message(buffer, client);

    // 클라이언트로부터 메시지 수신 및 전송
    while ((read_size = recv(client->socket, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        buffer[read_size] = '\0';

        //메시지 작성
        size_t message_len = strlen(client->name) + 2 + strlen(buffer) + 1;
        char *message = (char*)malloc(message_len);
        if (message != NULL)
        {
            snprintf(message, message_len, "%s: %s", client->name, buffer);
            printf("%s", message);
            send_message(message, client); // 다른 클라이언트에게 메시지 전송
            free(message);
        }
    }

    close(client->socket); // 클라이언트 소켓 닫기

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i] == client)
        {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    snprintf(buffer, sizeof(buffer), "------ 사용자 \"%s\" 퇴장. -----\n", client->name);
    printf("%s", buffer);
    send_message(buffer, client); // 다른 클라이언트에게 퇴장 알림 전송

    free(client); // 클라이언트 메모리 해제
    return NULL;
}

// 메시지 전송 함수
void send_message(char *message, client_t *exclude_client)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i] != NULL && clients[i] != exclude_client)
        {
            send(clients[i]->socket, message, strlen(message), 0); // 다른 클라이언트에게 메시지 전송
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 시그널 핸들러 함수(서버 종료 처리 함수)
void signal_handler(int sig)
{
    char c;
    printf("서버를 종료하시겠습니까? (y/n)\n");
    scanf(" %c", &c);

    if (c == 'y' || c == 'Y')
    {
        printf("서버 종료 중...\n");
        exit(0);
    }
    else
    {
        // 다른 입력은 아무런 동작도 하지 않고 함수를 종료
        return;
    }
}