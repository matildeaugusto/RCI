#ifndef UTILS_H
#define UTILS_H
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>

#include "udp_communication.h"
#include "server.h"

/* typedef struct {
    char *external_neighbor_ip;
    int external_neighbor_port;
    char *backup_node_ip;
    int backup_node_port;
    char **internal_neighbors_ips;
    int *internal_neighbors_ports;
    int internal_neighbors_count;
} ndn_node_info; */

bool isValidIpAddress(char *ipAddress);
bool isValidPort(char *port);

void show_help();

int is_first_connection(tcp_socket *server_info);

#endif