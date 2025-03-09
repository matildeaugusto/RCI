#ifndef EVENTS_H
#define EVENTS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>

#include "udp_communication.h"
#include "server.h"
#include "utils.h"

int wait_for_event(udp_comms *node_server_info, tcp_socket *tcp_server_info, int *internal_index);
int get_user_input(udp_comms *comms_info_to_node_server, tcp_socket *server_info, cache_info *ci, interest_table *it, int j);


#endif