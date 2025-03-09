#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_NODES 100
#define BUFFER_SIZE 1024

typedef struct {
    char net[4];
    char ip[16];
    char port[6];
} Node;

Node nodes[MAX_NODES];
int node_count = 0;  // Inicialmente, nenhum nó registrado

void handle_reg_message(const char *data, char *response) {
    char net[4], ip[16], port[6];
    if (sscanf(data, "REG %3s %15s %5s", net, ip, port) != 3) {
        strcpy(response, "ERRO: Mensagem REG inválida");
        printf("[DEBUG] Mensagem REG inválida recebida: %s\n", data);
        return;
    }

    printf("[DEBUG] Mensagem REG recebida: %s\n", data);

    if (node_count < MAX_NODES) {
        strcpy(nodes[node_count].net, net);
        strcpy(nodes[node_count].ip, ip);
        strcpy(nodes[node_count].port, port);
        node_count++;
        sprintf(response, "OKREG %s %s %s", net, ip, port);
        printf("[DEBUG] Nó adicionado com sucesso: %s %s %s\n", net, ip, port);
    } else {
        strcpy(response, "ERRO: Limite de nós atingido");
        printf("[DEBUG] Limite de nós atingido\n");
    }
}

void handle_unreg_message(const char *data, char *response) {
    char net[4], ip[16], port[6];
    if (sscanf(data, "UNREG %3s %15s %5s", net, ip, port) != 3) {
        strcpy(response, "ERRO: Mensagem UNREG inválida");
        return;
    }

    for (int i = 0; i < node_count; i++) {
        if (strcmp(nodes[i].net, net) == 0 &&
            strcmp(nodes[i].ip, ip) == 0 &&
            strcmp(nodes[i].port, port) == 0) {
            // Remove o nó trocando com o último e diminuindo o contador
            nodes[i] = nodes[node_count - 1];
            node_count--;
            sprintf(response, "UNREGOK %s %s %s", net, ip, port);
            return;
        }
    }
    strcpy(response, "ERRO: Nó não encontrado");
}

void handle_nodes_message(const char *data, char *response) {
    char net[4];
    if (sscanf(data, "NODES %3s", net) != 1) {
        strcpy(response, "ERRO: Mensagem NODES inválida");
        return;
    }

    int count = 0;
    sprintf(response, "NODESLIST %s\n", net);
    for (int i = 0; i < node_count; i++) {
        if (strcmp(nodes[i].net, net) == 0) {
            sprintf(response + strlen(response), "%s %s\n", nodes[i].ip, nodes[i].port);
            count++;
        }
    }
    if (count == 0) {
        //strcat(response, "ERRO: Nenhum nó encontrado para a rede especificada\n");
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Cria o socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(59001);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao ligar socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor de nós iniciado em 127.0.0.1:59001\n");

    // Adicionando 3 nós no servidor
    strcpy(nodes[node_count].net, "001");
    strcpy(nodes[node_count].ip, "127.0.0.1");
    strcpy(nodes[node_count].port, "50001");
    node_count++;

    strcpy(nodes[node_count].net, "001");
    strcpy(nodes[node_count].ip, "127.0.0.1");
    strcpy(nodes[node_count].port, "50002");
    node_count++;

    strcpy(nodes[node_count].net, "001");
    strcpy(nodes[node_count].ip, "127.0.0.1");
    strcpy(nodes[node_count].port, "50003");
    node_count++;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, BUFFER_SIZE);

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("Erro ao receber dados");
            continue;
        }

        printf("Recebido de %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        if (strncmp(buffer, "REG", 3) == 0) {
            handle_reg_message(buffer, response);
        } else if (strncmp(buffer, "UNREG", 5) == 0) {
            handle_unreg_message(buffer, response);
        } else if (strncmp(buffer, "NODES", 5) == 0) {
            handle_nodes_message(buffer, response);
        } else {
            strcpy(response, "ERRO: Mensagem desconhecida");
        }

        printf("Enviando resposta para %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), response);
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_len);
    }

    close(sockfd);
    return 0;
}
