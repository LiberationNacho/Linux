#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_client(int client_socket);
void send_message(char *message, int exclude_socket);
void signal_handler(int sig);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    signal(SIGINT, signal_handler);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);

    printf("Chat server started on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_socket;
                pthread_t tid;
                pthread_create(&tid, NULL, (void*)handle_client, (void*)&client_socket);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size;
    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        send_message(buffer, client_socket);
    }

    close(client_socket);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] != 0 && client_sockets[i] != exclude_socket) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void signal_handler(int sig) {
    printf("Shutting down server...\n");
    exit(0);
}

