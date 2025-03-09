#ifndef UDP_COMMUNICATION_H
#define UDP_COMMUNICATION_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/select.h>

#define MAX_SIZE 100

typedef struct {
    int fd;
    struct addrinfo *res;
} udp_comms;

typedef struct {
    int fd;                          // File descriptor para aceitar conexões
    struct addrinfo *res;            // Informações de endereço
    char ip[INET_ADDRSTRLEN];        // Nosso IP
    char port[6];                    // Nossa porta
    char id[4];                     // Nosso ID
    char net[4];                         // Valor de net

    // Informações do vizinho externo
    int fd_external;
    char external_ip[INET_ADDRSTRLEN];
    char external_port[6];
    char external_id[4];
    int n_external;

    // Informações do nó de salvaguarda (backup)
    int fd_backup;
    char backup_ip[INET_ADDRSTRLEN];
    char backup_port[6];
    char backup_id[4];

    // Informações dos vizinhos internos
    int fd_internals[10];            // Até 10 vizinhos internos
    char internals_ip[10][INET_ADDRSTRLEN];
    char internals_port[10][6];
    char internals_id[10][4];
    int n_internals;                // Número de vizinhos internos

} tcp_socket;

typedef struct cache_entry {
    char name[100];          // Nome do objeto
    char data[1024];         // Dados do objeto (simplificado como um array de caracteres)
    time_t timestamp;        // Timestamp da última vez que o objeto foi acessado
} cache_entry;

typedef struct cache_info {
    cache_entry entries[MAX_SIZE]; // Array de entradas de cache
    int size;                            // Número de entradas atualmente no cache
} cache_info;

typedef enum {WAITING, RESPONDED, CLOSED} interest_state;

typedef struct interest_entry {
    char name[100];                      // Nome do objeto procurado
    interest_state state[10];           // Estado de cada interface (espera, respondido, fechado)
    int interface_count;                // Número de interfaces envolvidas na busca
} interest_entry;

typedef struct interest_table {
    interest_entry entries[MAX_SIZE]; // Array de entradas de interesse
    int size;                                        // Número de entradas atualmente na tabela
} interest_table;

// routing tables
typedef struct t_routing_tables
{
    char routing_table[100][100][MAX_SIZE];
    char shortest_path_table[100][MAX_SIZE];
    char dispatch_table[100][4];
} routing_info;


int create_udp_socket(int *fd, struct addrinfo **res, char *ip, char *port);
void receive_udp_message(udp_comms *comms_info_to_node_server, tcp_socket *server_info);

void handle_nodes_message(char *response, tcp_socket *server_info);


#endif
