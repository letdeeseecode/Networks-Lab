#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 4096

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

int read_line(int sock, char *buf, size_t maxlen) {
    size_t i = 0;
    char c;
    while (i < maxlen - 1) {
        int n = recv(sock, &c, 1, 0);
        if (n <= 0) return -1;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return (int)i;
}


int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}


void download_file(const char *ip, int port, const char *server_file) {
    int sockfd = connect_to_server(ip, port);
    if (sockfd < 0) return;

    char cmd[BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "DOWNLOAD %s\n", server_file);
    send_all(sockfd, cmd, strlen(cmd));

    char line[BUF_SIZE];
    if (read_line(sockfd, line, sizeof(line)) <= 0) {
        printf("Failed to read response.\n");
        close(sockfd);
        return;
    }

    if (strncmp(line, "OK ", 3) != 0) {
        printf("Server error: %s\n", line);
        close(sockfd);
        return;
    }

    long filesize;
    sscanf(line + 3, "%ld", &filesize);
    printf("Downloading %s (%ld bytes) from server...\n", server_file, filesize);

    FILE *fp = fopen(server_file, "wb");
    if (!fp) {
        perror("fopen");
        close(sockfd);
        return;
    }

    char buffer[BUF_SIZE];
    long remaining = filesize;
    ssize_t n;

    clock_t start = clock();

    while (remaining > 0) {
        size_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
        n = recv(sockfd, buffer, to_read, 0);
        if (n <= 0) {
            perror("recv");
            break;
        }
        fwrite(buffer, 1, n, fp);
        remaining -= n;
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Download finished in %.6f seconds.\n", elapsed);

    fclose(fp);
    close(sockfd);
}


void upload_file(const char *ip, int port, const char *client_file) {
    int sockfd = connect_to_server(ip, port);
    if (sockfd < 0) return;

    FILE *fp = fopen(client_file, "rb");
    if (!fp) {
        perror("fopen");
        close(sockfd);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char cmd[BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "UPLOAD %s %ld\n", client_file, filesize);
    send_all(sockfd, cmd, strlen(cmd));

    char line[BUF_SIZE];
    if (read_line(sockfd, line, sizeof(line)) <= 0) {
        printf("Failed to read response.\n");
        fclose(fp);
        close(sockfd);
        return;
    }

    if (strncmp(line, "OK", 2) != 0) {
        printf("Server error: %s\n", line);
        fclose(fp);
        close(sockfd);
        return;
    }

    printf("Uploading %s (%ld bytes) to server...\n", client_file, filesize);

    char buffer[BUF_SIZE];
    size_t nread;
    long remaining = filesize;

    clock_t start = clock();

    while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (send_all(sockfd, buffer, nread) < 0) {
            perror("send");
            break;
        }
        remaining -= nread;
        if (remaining <= 0) break;
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Upload finished in %.6f seconds.\n", elapsed);

    fclose(fp);
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr,
                "Usage: %s <server_ip> <port> <server_file_to_download> <client_file_to_upload>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *server_file = argv[3];
    const char *client_file = argv[4];

    
    download_file(server_ip, port, server_file);

    
    upload_file(server_ip, port, client_file);

    return 0;
}
