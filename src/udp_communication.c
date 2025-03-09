#define _GNU_SOURCE

#include "udp_communication.h"

#define WAIT_TIME 5 

int create_udp_socket(int *fd, struct addrinfo **res, char *ip, char *port)
{

    struct addrinfo hints;
    int errcode;

    *fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (*fd == -1)
        return 0; // error

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    errcode = getaddrinfo(ip, port, &hints, res);
    if (errcode != 0)
        return 0; // error

    return 1;
}

void receive_udp_message(udp_comms *comms_info_to_node_server, tcp_socket *server_info) {
    char buff[1024];
    socklen_t addrlen = comms_info_to_node_server->res->ai_addrlen;

    if (recvfrom(comms_info_to_node_server->fd, buff, sizeof(buff), 0, comms_info_to_node_server->res->ai_addr, &addrlen) <= 0) {
        perror("Erro ao receber mensagem UDP");
        return;
    }

    if (strncmp(buff, "NODESLIST", 9) == 0) {
        handle_nodes_message(buff, server_info);
    } else {
        printf("Mensagem UDP desconhecida recebida: %s\n", buff);
    }
}

void handle_nodes_message(char *response, tcp_socket *server_info) {
    char ip[INET_ADDRSTRLEN];
    char port[6];
    char net[50];

    if (sscanf(response, "NODESLIST %49s %s %s", net, ip, port) == 3) {
        printf("Nó encontrado na rede %s: %s:%s\n", net, ip, port);
    } else {
        printf("Erro: Resposta do servidor de nós inválida.\n");
    }
}
