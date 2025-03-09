#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "utils.h"
#include "udp_communication.h"
#include "events.h"
#include "server.h"

tcp_socket *init_server(char *ip, char *port) {
    tcp_socket *server = (tcp_socket *)calloc(1, sizeof(tcp_socket));
    if (server == NULL) {
        perror("Falha ao alocar memória para o servidor");
        return NULL;
    }

    // Atribui valores de IP e Porta
    strncpy(server->port, port, sizeof(server->port) - 1);
    server->port[sizeof(server->port) - 1] = '\0';

    strncpy(server->ip, ip, sizeof(server->ip) - 1);
    server->ip[sizeof(server->ip) - 1] = '\0';

    // Inicializa as variáveis de controle de conexão
    server->n_internals = 0;
    server->n_external = 0;

    // Inicializa os arrays de IPs e portas dos vizinhos internos
    for (int i = 0; i < 10; i++) {
        server->fd_internals[i] = -1;
        server->internals_ip[i][0] = '\0';
        server->internals_port[i][0] = '\0';
    }

    // Inicializa o nó de salvaguarda
    server->backup_ip[0] = '\0';
    server->backup_port[0] = '\0';

    return server;
}

cache_info *init_cache(int size) {
    cache_info *ci = (cache_info *)calloc(1, sizeof(cache_info));
    if (ci == NULL) {
        return NULL;
    }
    ci->size = size;
    return ci;
}

interest_table *init_interest_table() {
    interest_table *it = (interest_table *)calloc(1, sizeof(interest_table));
    if (it == NULL) {
        return NULL;
    }
    it->size = 0;
    return it;
}

int main(int argc, char **argv) {
    char regIP[] = "127.0.0.1"; 
    char regUDP[] = "59001";          
    int successful = 1;               
    int event_id = 0;                 

    if (argc != 4 && argc != 6) {
        printf("Número errado de argumentos.\n");
        printf("Uso: ./ndn cache IP TCP [regIP regUDP]\n");
        return 0;
    }

    if (!isValidIpAddress(argv[2]) || !isValidPort(argv[3])) {
        printf("Endereço IP ou porta TCP inválidos.\n");
        return 0;
    }

    if (argc == 6)
    {
        if (isValidIpAddress(argv[4]) != true)
        {
            printf("Invalid IP address.\n");
            return 0;
        }
        else
        {
            strcpy(regIP, argv[4]);
        }

        if (isValidPort(argv[5]) != true)
        {
            printf("Invalid UDP port.\n");
            return 0;
        }
        else
        {
            strcpy(regUDP, argv[5]);
        }
    }

    udp_comms *node_server_info = (udp_comms *)calloc(1, sizeof(udp_comms));
    if (node_server_info == NULL) {
        printf("ERRO: calloc(node_server_info)\n");
        return 0;
    }

    if (create_udp_socket(&node_server_info->fd, &node_server_info->res, regIP, regUDP) == 0) {
        printf("ERRO: Não foi possível criar socket para o servidor de nós.\n");
        return 0;
    }

    tcp_socket *server_info = init_server(argv[2], argv[3]);
    if (server_info == NULL) {
        printf("ERRO: init_server()\n");
        free(node_server_info);
        return 0;
    }

    cache_info *ci = init_cache(atoi(argv[1]));
    interest_table *it = init_interest_table();
    if (ci == NULL || it == NULL) {
        printf("ERRO: init_cache() ou init_interest_table()\n");
        free(server_info);
        free(node_server_info);
        return EXIT_FAILURE;
    }

    if (open_tcp_socket(server_info) == 0) {
        printf("Erro ao criar servidor TCP, encerrando.\n");
        free(ci);
        free(it);
        free(server_info);
        free(node_server_info);
        return EXIT_FAILURE;
    }

    show_help();

    while (1) {
        successful = 1;
        int j = 0;
        int internal_index = -1; 
        event_id = wait_for_event(node_server_info, server_info, &internal_index);

        if (event_id == 1) {
            successful = get_user_input(node_server_info, server_info, ci, it, j);
        } else if (event_id == 2) {
            successful = accept_incoming_connection(server_info);
        } else if (event_id == 3) {
            successful = receive_tcp_message_from_out(server_info, ci, it);
        } else if (event_id == 4) {
            successful = receive_tcp_message_from_internal(server_info, ci, it, internal_index);
            
        } else if (event_id == 5) {
            successful = receive_tcp_message_from_external(server_info, ci, it, node_server_info, j, internal_index);
        }

        if (successful == 2) {
            break;
        } else if (successful == 0) {
            // printf("[DEBUG] Ocorreu um erro! event_id: %i \n", event_id);
        }
    }

    freeaddrinfo(node_server_info->res);
    freeaddrinfo(server_info->res);
    free(node_server_info);
    free(server_info);
    free(ci);
    free(it);
    return EXIT_SUCCESS;
}