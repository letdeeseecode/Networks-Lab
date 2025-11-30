#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>

#define BUF_SIZE 1024


double evaluate_expression(const char *expr, int *ok) {
    double x, y, result;
    *ok = 1;

    
    if (sscanf(expr, "sin(%lf)", &x) == 1) {
        result = sin(x);
    } else if (sscanf(expr, "cos(%lf)", &x) == 1) {
        result = cos(x);
    } else if (sscanf(expr, "tan(%lf)", &x) == 1) {
        result = tan(x);
    } else if (sscanf(expr, "sqrt(%lf)", &x) == 1) {
        if (x < 0) { *ok = 0; return 0.0; }
        result = sqrt(x);
    } else if (sscanf(expr, "inv(%lf)", &x) == 1) { // 1/x
        if (x == 0) { *ok = 0; return 0.0; }
        result = 1.0 / x;

    
    } else if (sscanf(expr, "%lf+%lf", &x, &y) == 2) {
        result = x + y;
    } else if (sscanf(expr, "%lf-%lf", &x, &y) == 2) {
        result = x - y;
    } else if (sscanf(expr, "%lf*%lf", &x, &y) == 2) {
        result = x * y;
    } else if (sscanf(expr, "%lf/%lf", &x, &y) == 2) {
        if (y == 0) { *ok = 0; return 0.0; }
        result = x / y;
    } else {
        
        *ok = 0;
        result = 0.0;
    }

    return result;
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cli_len;
    char buffer[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Calculator Server listening on port %d...\n", port);

    while (1) {
        cli_len = sizeof(cliaddr);
        memset(buffer, 0, BUF_SIZE);

        
        ssize_t n = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0,
                             (struct sockaddr *)&cliaddr, &cli_len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[n] = '\0';

        
        char *sep = strchr(buffer, '|');
        if (!sep) {
            
            continue;
        }

        *sep = '\0';
        char *seq_str = buffer;
        char *expr = sep + 1;

        int seq = atoi(seq_str);
        printf("Received from client [seq=%d]: %s\n", seq, expr);

        int ok = 0;
        double result = evaluate_expression(expr, &ok);

        char reply[BUF_SIZE];
        if (ok) {
            snprintf(reply, sizeof(reply), "%d|OK|%lf", seq, result);
        } else {
            snprintf(reply, sizeof(reply), "%d|ERR|Invalid expression or math error", seq);
        }

        
        sendto(sockfd, reply, strlen(reply), 0,
               (struct sockaddr *)&cliaddr, cli_len);

        printf("Sent to client [seq=%d]: %s\n", seq, reply);
    }

    close(sockfd);
    return 0;
}
