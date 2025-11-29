#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_FRUITS 5
#define MAX_CUSTOMERS 100
#define BUF_SIZE 2048

typedef struct {
    char name[32];
    int quantity;
    char last_sold[64];  
} Fruit;

typedef struct {
    char id[64];         
} Customer;

Fruit inventory[MAX_FRUITS] = {
    {"apple", 50, "never"},
    {"banana", 30, "never"},
    {"orange", 75, "never"},
    {"mango", 45, "never"},
    {"grape", 40, "never"}
};

Customer customers[MAX_CUSTOMERS];
int customer_count = 0;


void add_customer_if_new(const char *client_id) {
    for (int i = 0; i < customer_count; i++) {
        if (strcmp(customers[i].id, client_id) == 0) {
            return;
        }
    }
    if (customer_count < MAX_CUSTOMERS) {
        strncpy(customers[customer_count].id, client_id,
                sizeof(customers[customer_count].id) - 1);
        customers[customer_count].id[sizeof(customers[customer_count].id) - 1] = '\0';
        customer_count++;
    }
}

void print_customers() {
    printf("\nCustomers who did transactions:\n");
    for (int i = 0; i < customer_count; i++) {
        printf("%d) %s\n", i + 1, customers[i].id);
    }
    printf("Total unique customers: %d\n\n", customer_count);
}


int append_stock_info(char *buf, int offset, int max_size) {
    offset += snprintf(buf + offset, max_size - offset,
                       "Current stock:\n");
    for (int i = 0; i < MAX_FRUITS; i++) {
        offset += snprintf(buf + offset, max_size - offset,
                           "%s: qty=%d, last_sold=%s\n",
                           inventory[i].name,
                           inventory[i].quantity,
                           inventory[i].last_sold);
        if (offset >= max_size) break;
    }
    return offset;
}

int main() {
    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUF_SIZE];

    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        exit(1);
    }

    printf("UDP Fruit Server running on port %d...\n", PORT);

    while (1) {
        client_len = sizeof(client_addr);
        memset(buffer, 0, sizeof(buffer));

        int n = recvfrom(server_sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

       
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        int port = ntohs(client_addr.sin_port);
        char client_id[64];
        snprintf(client_id, sizeof(client_id), "%s:%d", ip_str, port);

        printf("Request from %s: '%s'\n", client_id, buffer);

       
        char *p = strchr(buffer, '\n');
        if (p) *p = '\0';

        char reply[BUF_SIZE];
        int offset = 0;
        memset(reply, 0, sizeof(reply));

      
        if (strcasecmp(buffer, "stock") == 0) {
            offset = append_stock_info(reply, offset, sizeof(reply));
            sendto(server_sock, reply, offset, 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

       
        if (strcasecmp(buffer, "exit") == 0) {
            offset += snprintf(reply + offset, sizeof(reply) - offset,
                               "Server: 'exit' received from %s (UDP no connection).\n",
                               client_id);
            offset = append_stock_info(reply, offset, sizeof(reply));
            sendto(server_sock, reply, offset, 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

       
        char fruit_name[32];
        int qty;
        if (sscanf(buffer, "%31s %d", fruit_name, &qty) != 2) {
            snprintf(reply, sizeof(reply),
                     "Invalid format. Use: <fruit_name> <quantity> or 'exit' or 'stock'\n");
            sendto(server_sock, reply, strlen(reply), 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

        int index = -1;
        for (int i = 0; i < MAX_FRUITS; i++) {
            if (strcasecmp(inventory[i].name, fruit_name) == 0) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            offset += snprintf(reply + offset, sizeof(reply) - offset,
                               "REGRET: Fruit '%s' not found.\n", fruit_name);
        } else if (qty <= 0) {
            offset += snprintf(reply + offset, sizeof(reply) - offset,
                               "REGRET: Quantity must be positive.\n");
        } else if (inventory[index].quantity < qty) {
            offset += snprintf(reply + offset, sizeof(reply) - offset,
                               "REGRET: Only %d '%s' available.\n",
                               inventory[index].quantity,
                               inventory[index].name);
        } else {
           
            inventory[index].quantity -= qty;

            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            strftime(inventory[index].last_sold,
                     sizeof(inventory[index].last_sold),
                     "%Y-%m-%d %H:%M:%S", t);

            offset += snprintf(reply + offset, sizeof(reply) - offset,
                               "SUCCESS: Sold %d '%s'. Remaining: %d\n",
                               qty, inventory[index].name, inventory[index].quantity);

            add_customer_if_new(client_id);
            print_customers();
        }

        offset += snprintf(reply + offset, sizeof(reply) - offset,
                           "Unique customers so far: %d\n", customer_count);
        offset = append_stock_info(reply, offset, sizeof(reply));

        sendto(server_sock, reply, offset, 0,
               (struct sockaddr *)&client_addr, client_len);
    }

    close(server_sock);
    return 0;
}
