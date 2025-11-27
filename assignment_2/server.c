#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_FRUITS 5
#define MAX_CUSTOMERS 100
#define BUF_SIZE 1024

typedef struct {
    char name[32];
    int quantity;
    char last_sold[64];
} Fruit;

typedef struct {
    char id[64];  
} Customer;

Fruit inventory[MAX_FRUITS] = {
    {"apple", 20, "never"},
    {"banana", 30, "never"},
    {"orange", 15, "never"},
    {"mango", 25, "never"},
    {"grape", 40, "never"}
};

Customer customers[MAX_CUSTOMERS];
int customer_count = 0;

void add_customer_if_new(const char *client_id) {
    int i;
    for (i = 0; i < customer_count; i++) {
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
    int i;
    printf("\nCustomers who did transactions:\n");
    for (i = 0; i < customer_count; i++) {
        printf("%d) %s\n", i + 1, customers[i].id);
    }
    printf("Total unique customers: %d\n\n", customer_count);
}

void send_stock_info(int client_sock) {
    char buffer[BUF_SIZE];
    int offset = 0;
    int i;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "Current stock:\n");
    for (i = 0; i < MAX_FRUITS; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "%s: qty=%d, last_sold=%s\n",
                           inventory[i].name,
                           inventory[i].quantity,
                           inventory[i].last_sold);
        if (offset >= (int)sizeof(buffer)) break;
    }

    send(client_sock, buffer, strlen(buffer), 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buffer[BUF_SIZE];

  
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        exit(1);
    }


    if (listen(server_sock, 5) < 0) {
        perror("listen");
        close(server_sock);
        exit(1);
    }

    printf("Fruit server running on port %d...\n", PORT);

    while (1) {
        addr_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        int port = ntohs(client_addr.sin_port);
        char client_id[64];
        snprintf(client_id, sizeof(client_id), "%s:%d", ip_str, port);

        printf("New client connected: %s\n", client_id);

      
        send_stock_info(client_sock);

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) {
                printf("Client %s disconnected.\n", client_id);
                break;
            }
            buffer[n] = '\0';

            
            char *p = strchr(buffer, '\n');
            if (p) *p = '\0';

            if (strcasecmp(buffer, "exit") == 0) {
                printf("Client %s sent exit.\n", client_id);
                break;
            }

            char fruit_name[32];
            int qty;
            if (sscanf(buffer, "%31s %d", fruit_name, &qty) != 2) {
                const char *msg = "Use format: <fruit_name> <quantity> or 'exit'\n";
                send(client_sock, msg, strlen(msg), 0);
                continue;
            }

            
            int i, index = -1;
            for (i = 0; i < MAX_FRUITS; i++) {
                if (strcasecmp(inventory[i].name, fruit_name) == 0) {
                    index = i;
                    break;
                }
            }

            char reply[BUF_SIZE];
            memset(reply, 0, sizeof(reply));

            if (index == -1) {
                snprintf(reply, sizeof(reply),
                         "REGRET: Fruit '%s' not found.\n", fruit_name);
            } else if (qty <= 0) {
                snprintf(reply, sizeof(reply),
                         "REGRET: Quantity must be positive.\n");
            } else if (inventory[index].quantity < qty) {
                snprintf(reply, sizeof(reply),
                         "REGRET: Only %d '%s' available.\n",
                         inventory[index].quantity, inventory[index].name);
            } else {
                
                inventory[index].quantity -= qty;

                
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                strftime(inventory[index].last_sold,
                         sizeof(inventory[index].last_sold),
                         "%Y-%m-%d %H:%M:%S", t);

                snprintf(reply, sizeof(reply),
                         "SUCCESS: Sold %d '%s'. Remaining: %d\n",
                         qty, inventory[index].name, inventory[index].quantity);

                
                add_customer_if_new(client_id);
                print_customers();
            }

            
            int len = strlen(reply);
            snprintf(reply + len, sizeof(reply) - len,
                     "Unique customers so far: %d\n\n", customer_count);

            send(client_sock, reply, strlen(reply), 0);

            
            send_stock_info(client_sock);
        }

        close(client_sock);
    }

    close(server_sock);
    return 0;
}
