#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 2048

int client_socket;
void *receive_handler(void *socket_desc);
void signal_handler(int sig);

int main() {
    struct sockaddr_in server_addr;
    pthread_t receive_thread;

    signal(SIGINT, signal_handler);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    pthread_create(&receive_thread, NULL, receive_handler, (void*)&client_socket);

    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        send(client_socket, message, strlen(message), 0);
    }

    close(client_socket);
    return 0;
}

void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;
    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);
    }
    return 0;
}

void signal_handler(int sig) {
    printf("Disconnecting from server...\n");
    close(client_socket);
    exit(0);
}
