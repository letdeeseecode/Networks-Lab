#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>   
#include <netinet/tcp.h>   

#define BUF_SIZE 65535


void print_ip_header(struct iphdr *iph) {
    struct in_addr src_addr, dst_addr;
    src_addr.s_addr = iph->saddr;
    dst_addr.s_addr = iph->daddr;

    printf("--------------- IP HEADER ---------------\n");
    printf("Version        : %d\n", iph->version);
    printf("Header Length  : %d bytes\n", iph->ihl * 4);
    printf("Total Length   : %d bytes\n", ntohs(iph->tot_len));
    printf("TTL            : %d\n", iph->ttl);
    printf("Protocol       : %d (TCP is 6)\n", iph->protocol);
    printf("Source IP      : %s\n", inet_ntoa(src_addr));
    printf("Destination IP : %s\n", inet_ntoa(dst_addr));
}


void print_tcp_header(struct tcphdr *tcph) {
    printf("-------------- TCP HEADER --------------\n");
    printf("Source Port      : %u\n", ntohs(tcph->source));
    printf("Destination Port : %u\n", ntohs(tcph->dest));
    printf("Sequence Number  : %u\n", ntohl(tcph->seq));
    printf("Ack Number       : %u\n", ntohl(tcph->ack_seq));
    printf("Header Length    : %d bytes\n", tcph->doff * 4);
    printf("Flags: \n");
    printf("  URG: %d, ACK: %d, PSH: %d, RST: %d, SYN: %d, FIN: %d\n",
           tcph->urg, tcph->ack, tcph->psh, tcph->rst, tcph->syn, tcph->fin);
    printf("Window Size      : %u\n", ntohs(tcph->window));
}


void print_data(unsigned char *data, int size) {
    int i, j;
    printf("--------------- DATA (%d bytes) ---------------\n", size);
    for (i = 0; i < size; i += 16) {
        
        printf("%04x  ", i);
        for (j = 0; j < 16 && i + j < size; j++) {
            printf("%02x ", data[i + j]);
        }
        
        for (; j < 16; j++) {
            printf("   ");
        }
        printf(" ");

       
        for (j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = data[i + j];
            if (c >= 32 && c <= 126)
                printf("%c", c);
            else
                printf(".");
        }
        printf("\n");
    }
    printf("-----------------------------------------\n\n");
}

int main() {
    int sockfd;
    ssize_t data_size;
    struct sockaddr saddr;
    socklen_t saddr_len;
    unsigned char *buffer;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Socket creation failed (need sudo/root?)");
        exit(EXIT_FAILURE);
    }

    printf("TCP Traffic Analyzer started. Listening for incoming TCP packets...\n");
    printf("Press Ctrl+C to stop.\n\n");

    buffer = (unsigned char *) malloc(BUF_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        saddr_len = sizeof(saddr);
     
        data_size = recvfrom(sockfd,
                             buffer,
                             BUF_SIZE,
                             0,
                             &saddr,
                             &saddr_len);
        if (data_size < 0) {
            perror("recvfrom failed");
            break;
        }

      
        struct iphdr *iph = (struct iphdr *) buffer;

       
        if (iph->protocol != IPPROTO_TCP) {
            continue;
        }

        int ip_header_len = iph->ihl * 4;

        
        struct tcphdr *tcph = (struct tcphdr *) (buffer + ip_header_len);
        int tcp_header_len = tcph->doff * 4;

        
        int header_total_len = ip_header_len + tcp_header_len;
        int payload_len = ntohs(iph->tot_len) - header_total_len;

        unsigned char *payload = buffer + header_total_len;

        
        print_ip_header(iph);
        print_tcp_header(tcph);

        if (payload_len > 0) {
            print_data(payload, payload_len);
        } else {
            printf("No TCP payload (control packet).\n\n");
        }
    }

    free(buffer);
    close(sockfd);
    return 0;
}
