#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define PACKET_SIZE 4096


#define SRC_IP     "10.0.0.1"
#define DST_IP     "10.0.0.2"
#define SRC_PORT   8080
#define DST_PORT   7070
#define ROLL_STR   "CSM24016"


unsigned short checksum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    unsigned short answer;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (unsigned short)~sum;
    return answer;
}


struct pseudo_header {
    unsigned int src;
    unsigned int dst;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_len;
};

int main() {
    const char *src_ip = SRC_IP;
    const char *dst_ip = DST_IP;
    int src_port = SRC_PORT;
    int dst_port = DST_PORT;
    const char *payload = ROLL_STR;
    int payload_len = strlen(payload);

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);

    struct iphdr *iph  = (struct iphdr *) packet;
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
    char *data = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
    memcpy(data, payload, payload_len);

   
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + payload_len);
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = inet_addr(src_ip);
    iph->daddr = inet_addr(dst_ip);
    iph->check = 0;   

    
    tcph->source = htons(src_port);
    tcph->dest = htons(dst_port);
    tcph->seq = htonl(1);
    tcph->ack_seq = 0;
    tcph->doff = 5;       
    tcph->syn = 1;        
    tcph->ack = 0;
    tcph->fin = 0;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->urg = 0;
    tcph->window = htons(65535);
    tcph->check = 0;     
    tcph->urg_ptr = 0;

    
    struct pseudo_header psh;
    psh.src = iph->saddr;
    psh.dst = iph->daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_len = htons(sizeof(struct tcphdr) + payload_len);

    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payload_len;
    char *pseudogram = malloc(psize);
    if (!pseudogram) {
        perror("malloc");
        close(sock);
        return 1;
    }

    memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header),
           tcph, sizeof(struct tcphdr) + payload_len);

    tcph->check = checksum((unsigned short *)pseudogram, psize);
    free(pseudogram);

    
    iph->check = checksum((unsigned short *)packet, sizeof(struct iphdr));

    
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = iph->daddr;

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    printf("\nSending TCP packet with hard-coded values:\n");
    printf("  SRC_IP   = %s\n", src_ip);
    printf("  DST_IP   = %s\n", dst_ip);
    printf("  SRC_PORT = %d\n", src_port);
    printf("  DST_PORT = %d\n", dst_port);
    printf("  PAYLOAD  = '%s'\n\n", payload);

    ssize_t sent = sendto(sock,
                          packet,
                          sizeof(struct iphdr) + sizeof(struct tcphdr) + payload_len,
                          0,
                          (struct sockaddr *)&sin,
                          sizeof(sin));

    if (sent < 0) {
        perror("sendto");
    } else {
        printf("TCP packet sent successfully (%zd bytes).\n", sent);
    }

    close(sock);
    return 0;
}
