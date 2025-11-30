#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>  

#define BUF_SIZE   1024
#define EXPR_SIZE  256      
#define MAX_RETRIES 3

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;
    char sendbuf[BUF_SIZE], recvbuf[BUF_SIZE];
    socklen_t serv_len;
    int seq = 1;   

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    
    struct timeval tv;
    tv.tv_sec = 2;   
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    serv_len = sizeof(servaddr);

    printf("UDP Calculator Client started.\n");
    printf("Type expressions like: sin(1.57), 10+20, 3.5*2, inv(5)\n");
    printf("Type 'quit' to exit.\n\n");

    while (1) {
        char expr[EXPR_SIZE];

        printf("Enter expression: ");
        fflush(stdout);

        if (!fgets(expr, sizeof(expr), stdin)) {
            break;
        }

        
        size_t len = strlen(expr);
        if (len > 0 && expr[len - 1] == '\n') {
            expr[len - 1] = '\0';
        }

        if (strcmp(expr, "quit") == 0) {
            break;
        }

        if (strlen(expr) == 0) {
            continue;
        }

        
        snprintf(sendbuf, sizeof(sendbuf), "%d|%s", seq, expr);

        int retries = 0;
        int got_reply = 0;

        while (retries < MAX_RETRIES && !got_reply) {
            
            ssize_t sent = sendto(sockfd, sendbuf, strlen(sendbuf), 0,
                                  (struct sockaddr *)&servaddr, serv_len);
            if (sent < 0) {
                perror("sendto failed");
                break;
            }

            
            ssize_t n = recvfrom(sockfd, recvbuf, sizeof(recvbuf) - 1, 0,
                                 (struct sockaddr *)&servaddr, &serv_len);

            if (n < 0) {
               
                retries++;
                printf("No response (timeout). Possible packet loss. Retrying %d/%d...\n",
                       retries, MAX_RETRIES);
                continue;
            }

            recvbuf[n] = '\0';

           
            char *p1 = strchr(recvbuf, '|');
            if (!p1) continue;
            *p1 = '\0';
            int r_seq = atoi(recvbuf);

            if (r_seq != seq) {
                
                printf("Received mismatched seq (got %d, expected %d)\n", r_seq, seq);
                continue;
            }

            char *p2 = strchr(p1 + 1, '|');
            if (!p2) continue;
            *p2 = '\0';

            char *status = p1 + 1;
            char *payload = p2 + 1;

            if (strcmp(status, "OK") == 0) {
                double result = atof(payload);
                printf("Result = %lf\n", result);
            } else {
                printf("Server error: %s\n", payload);
            }

            got_reply = 1;
        }

        if (!got_reply) {
            printf("Server did not respond after %d retries. Packet likely lost.\n\n",
                   MAX_RETRIES);
        }

        seq++;  
    }

    close(sockfd);
    return 0;
}
