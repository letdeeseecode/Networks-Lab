#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define ETHER_ADDR_LEN 6


struct ethernet_header {
    u_char  dst_mac[ETHER_ADDR_LEN];
    u_char  src_mac[ETHER_ADDR_LEN];
    u_short ether_type;
};


struct arp_header {
    u_short htype;
    u_short ptype;
    u_char  hlen;
    u_char  plen;
    u_short oper;
};


int seen_l2_eth  = 0;
int seen_l3_ipv4 = 0;
int seen_l3_arp  = 0;
int seen_l4_icmp = 0;
int seen_l4_tcp  = 0;
int seen_l4_udp  = 0;

void print_packet_flow(const struct pcap_pkthdr *header, const u_char *packet)
{
    const struct ethernet_header *eth;
    const struct ip *ip_hdr;
    const struct arp_header *arp_hdr;

    double ts = header->ts.tv_sec + header->ts.tv_usec / 1000000.0;

    eth = (const struct ethernet_header *) packet;
    u_short ether_type = ntohs(eth->ether_type);

    const u_char *payload = packet + sizeof(struct ethernet_header);

    seen_l2_eth = 1;

    printf("[%.6f] ", ts);
    printf("ETH -> ");

    if (ether_type == 0x0800) {
        ip_hdr = (const struct ip *) payload;
        seen_l3_ipv4 = 1;
        printf("IP -> ");

        switch (ip_hdr->ip_p) {
        case IPPROTO_ICMP:
            seen_l4_icmp = 1;
            printf("ICMP (Ping)");
            printf(" [Packet Size: %u]", header->len);
            break;

        case IPPROTO_TCP:
            seen_l4_tcp = 1;
            printf("TCP [Packet Size: %u]", header->len);
            break;

        case IPPROTO_UDP:
            seen_l4_udp = 1;
            printf("UDP [Packet Size: %u]", header->len);
            break;

        default:
            printf("Unknown Transport (%d)", ip_hdr->ip_p);
            printf(" [Packet Size: %u]", header->len);
            break;
        }

    } else if (ether_type == 0x0806) {
        seen_l3_arp = 1;
        arp_hdr = (const struct arp_header *) payload;
        u_short oper = ntohs(arp_hdr->oper);

        printf("ARP -> ");
        if (oper == 1)
            printf("Request");
        else if (oper == 2)
            printf("Reply");
        else
            printf("Operation %d", oper);

    } else {
        printf("Unknown L2 Protocol");
    }

    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pcap-file>\n", argv[0]);
        return 1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    const u_char *packet;
    struct pcap_pkthdr header;

    int packet_count = 0;   

    handle = pcap_open_offline(argv[1], errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open pcap file %s: %s\n", argv[1], errbuf);
        return 2;
    }

    printf("---------------- TIME DIAGRAM ----------------\n");
    printf("Timestamp           | Protocol Flow\n");
    printf("----------------------------------------------\n");

    while ((packet = pcap_next(handle, &header)) != NULL) {
        print_packet_flow(&header, packet);
        packet_count++;  
    }

    pcap_close(handle);

    printf("----------------------------------------------\n");
    printf("Total Packets Processed: %d\n\n", packet_count);  

    printf("SUMMARY OF DETECTED PROTOCOLS:\n");

    printf("Layer 2 (Data Link): ");
    if (seen_l2_eth) printf("Ethernet\n");
    else printf("None\n");

    printf("Layer 3 (Network):   ");
    int first = 1;
    if (seen_l3_ipv4) { printf("IPv4"); first = 0; }
    if (seen_l3_arp)  { if (!first) printf(" "); printf("ARP"); first = 0; }
    if (first) printf("None");
    printf("\n");

    printf("Layer 4 (Transport): ");
    first = 1;
    if (seen_l4_icmp) { printf("ICMP"); first = 0; }
    if (seen_l4_tcp)  { if (!first) printf(" "); printf("TCP"); first = 0; }
    if (seen_l4_udp)  { if (!first) printf(" "); printf("UDP"); first = 0; }
    if (first) printf("None");
    printf("\n");

    return 0;
}
