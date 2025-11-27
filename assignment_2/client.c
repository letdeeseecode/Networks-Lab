#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 2048

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }


    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

  
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("Connected to server %s:%s\n", argv[1], argv[2]);

    
    memset(buffer, 0, sizeof(buffer));
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("\n--- Initial Stock Information ---\n%s\n", buffer);
    }

    while (1) {
        char fruit[32];
        int qty;

        printf("\nEnter fruit name (or 'exit' to quit): ");
        if (scanf("%31s", fruit) != 1) {
            break;
        }

        if (strcasecmp(fruit, "exit") == 0) {
            // Tell server we are exiting
            snprintf(buffer, sizeof(buffer), "exit\n");
            send(sock, buffer, strlen(buffer), 0);
            break;
        }

        printf("Enter quantity: ");
        if (scanf("%d", &qty) != 1) {
            break;
        }

        
        snprintf(buffer, sizeof(buffer), "%s %d\n", fruit, qty);
        send(sock, buffer, strlen(buffer), 0);


        memset(buffer, 0, sizeof(buffer));
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("Server closed connection.\n");
            break;
        }
        buffer[n] = '\0';

        printf("\n--- Server Response ---\n%s\n", buffer);
    }

    close(sock);
    return 0;
}
