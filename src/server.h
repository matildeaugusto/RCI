#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "udp_communication.h"
#include "utils.h"

int open_tcp_socket(tcp_socket *server_info);
int accept_incoming_connection(tcp_socket *server_info);

int receive_tcp_message_from_out(tcp_socket *server_info, cache_info *ci, interest_table *it);
int receive_tcp_message_from_external(tcp_socket *server_info, cache_info *ci, interest_table *it, udp_comms *node_server_info, int j, int internal_index);
int receive_tcp_message_from_internal(tcp_socket *server_info, cache_info *ci, interest_table *it, int internal_index);

void handle_interest_message(char *name, tcp_socket *server_info, cache_info *ci, interest_table *it);
void handle_object_message(char *name, char *data, tcp_socket *server_info, cache_info *ci, interest_table *it);
void handle_no_object_message(char *name, tcp_socket *server_info, cache_info *ci, interest_table *it);

void send_object_message(char *name, char *data, tcp_socket *server_info);
void send_interest_message(char *name, int fd);
void send_no_object_message(char *name, tcp_socket *server_info);

int create_object(char *name, cache_info *ci);
int delete_object(char *name, cache_info *ci);
int retrieve_object(char *name, tcp_socket *server_info, cache_info *ci, interest_table *it);
void show_names(cache_info *ci);
void show_interest_table(interest_table *it);
int leave_network(udp_comms *comms_info_to_node_server, char *net, tcp_socket *server_info, cache_info *ci, interest_table *it);
void exit_application(udp_comms *comms_info_to_node_server, char *net, tcp_socket *server_info, cache_info *ci, interest_table *it);

int join(udp_comms *comms_info_to_node_server, tcp_socket *server_info, int j);
int direct_join(char *net, char *connectIP, char *connectPort, tcp_socket *server_info);
int create_tcp_connection(char *ip, char *port, tcp_socket *server_info);
void show_topology(tcp_socket *server_info);

void send_join_message(char *new_node_ip, char *new_node_port, tcp_socket *server_info);
void handle_join_message(char *new_node_ip, char *new_node_port, tcp_socket *server_info, int internal_index);

void send_safe_message(tcp_socket *server_info, int internal_index);
void handle_backup_message(char *backup_ip, char *backup_port, tcp_socket *server_info);

void send_reg_message(udp_comms *comms_info_to_node_server, char *ip, char *port,  char *net);
// void send_unreg_message(udp_comms *comms_info_to_node_server, char *net, char *ip, char *port);


void receive_regok_message(udp_comms *node_server_info);

void handle_join_message_j(char *new_node_ip, char *new_node_port, tcp_socket *server_info, int internal_index);
void send_safe_message_j(tcp_socket *server_info, int i);

int receive_safe_message(int fd);

#endif
