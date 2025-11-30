#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 4096


int read_line(int sock, char *buf, size_t maxlen) {
    size_t i = 0;
    char c;
    while (i < maxlen - 1) {
        int n = recv(sock, &c, 1, 0);
        if (n <= 0) return -1; // error or closed
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return (int)i;
}


int send_all(int sock, const void *buf, size_t len) {
    size_t total = 0;
    const char *p = buf;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return 0;
}

void handle_client(int client_fd) {
    char line[BUF_SIZE];

    
    if (read_line(client_fd, line, sizeof(line)) <= 0) {
        close(client_fd);
        return;
    }

    printf("Client command: %s\n", line);

    
    if (strncmp(line, "DOWNLOAD ", 9) == 0) {
        char *filename = line + 9;

        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            char msg[] = "ERR Cannot open file\n";
            send_all(client_fd, msg, strlen(msg));
            perror("fopen");
            close(client_fd);
            return;
        }

        
        fseek(fp, 0L, SEEK_END);
        long filesize = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        
        char header[128];
        snprintf(header, sizeof(header), "OK %ld\n", filesize);
        send_all(client_fd, header, strlen(header));

        char buffer[BUF_SIZE];
        size_t nread;
        long remaining = filesize;

        printf("Starting DOWNLOAD of %s (%ld bytes)\n", filename, filesize);

        
        clock_t start = clock();

        while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            if (send_all(client_fd, buffer, nread) < 0) {
                perror("send");
                break;
            }
            remaining -= nread;
            if (remaining <= 0) break;
        }

        
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("DOWNLOAD to client finished in %.6f seconds\n", elapsed);

        fclose(fp);
    }
    
    else if (strncmp(line, "UPLOAD ", 7) == 0) {
        char filename[256];
        long filesize;
        if (sscanf(line + 7, "%255s %ld", filename, &filesize) != 2) {
            char msg[] = "ERR Invalid UPLOAD command\n";
            send_all(client_fd, msg, strlen(msg));
            close(client_fd);
            return;
        }

        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            char msg[] = "ERR Cannot create file\n";
            send_all(client_fd, msg, strlen(msg));
            perror("fopen");
            close(client_fd);
            return;
        }

        
        char okmsg[] = "OK\n";
        send_all(client_fd, okmsg, strlen(okmsg));

        char buffer[BUF_SIZE];
        long remaining = filesize;
        ssize_t n;

        printf("Starting UPLOAD of %s (%ld bytes)\n", filename, filesize);

       
        clock_t start = clock();

        while (remaining > 0) {
            size_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
            n = recv(client_fd, buffer, to_read, 0);
            if (n <= 0) {
                perror("recv");
                break;
            }
            fwrite(buffer, 1, n, fp);
            remaining -= n;
        }

        
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("UPLOAD from client finished in %.6f seconds\n", elapsed);

        fclose(fp);
    } else {
        char msg[] = "ERR Unknown command\n";
        send_all(client_fd, msg, strlen(msg));
    }

    close(client_fd);
}

int main(int argc, char *argv[]) {
    int sockfd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("File server listening on port %d...\n", port);

    while (1) {
        client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("Client connected.\n");
        handle_client(client_fd);
        printf("Client disconnected.\n");
    }

    close(sockfd);
    return 0;
}
