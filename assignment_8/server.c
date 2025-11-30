#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUF_SIZE 1024

int clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;


void write_log(const char *msg) {
    pthread_mutex_lock(&log_mutex);
    FILE *fp = fopen("log.txt", "a");
    if (fp == NULL) {
        perror("log.txt");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp, "[%02d:%02d:%02d] %s\n",
            t->tm_hour, t->tm_min, t->tm_sec, msg);
    fclose(fp);
    pthread_mutex_unlock(&log_mutex);
}


void broadcast(char *msg, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0 && clients[i] != sender_fd) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}


void *client_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUF_SIZE];

    while (1) {
        memset(buffer, 0, BUF_SIZE);

        int read_size = recv(sock, buffer, BUF_SIZE, 0);
        if (read_size <= 0)
            break;

        buffer[read_size] = '\0';

        
        write_log(buffer);

        
        broadcast(buffer, sock);
    }

    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == sock) {
            clients[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int server_fd, newfd;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_fd, 10);
    printf("Chat server started on port %d...\n", port);

    while ((newfd = accept(server_fd, (struct sockaddr *)&client, &c))) {
        printf("Client connected!\n");

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == 0) {
                clients[i] = newfd;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        
        if (pthread_create(&thread_id, NULL, client_handler, (void *)&newfd) < 0) {
            perror("Thread creation failed");
            return 1;
        }

        pthread_detach(thread_id);
    }

    return 0;
}
