#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024

int sockfd;

void *recv_handler(void *arg) {
    char buffer[BUF_SIZE];

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        int n = recv(sockfd, buffer, BUF_SIZE, 0);
        if (n <= 0)
            break;

        printf("\n%s\n> ", buffer);
        fflush(stdout);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server;
    pthread_t recv_thread;
    char message[BUF_SIZE];

    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to chat server!\n");

    pthread_create(&recv_thread, NULL, recv_handler, NULL);

    while (1) {
        printf("> ");
        fflush(stdout);

        fgets(message, BUF_SIZE, stdin);
        if (strcmp(message, "quit\n") == 0)
            break;

        send(sockfd, message, strlen(message), 0);
    }

    close(sockfd);
    return 0;
}
