#include "events.h"

int wait_for_event(udp_comms *node_server_info, tcp_socket *tcp_server_info, int *internal_index) {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000; 
    fd_set rfds;
    FD_ZERO(&rfds);
    int maxfd = tcp_server_info->fd;

    FD_SET(0, &rfds);

    if (tcp_server_info->fd != -1) {
        FD_SET(tcp_server_info->fd, &rfds);
        if (tcp_server_info->fd > maxfd) maxfd = tcp_server_info->fd;
    }

    if (tcp_server_info->fd_external != -1) {
        FD_SET(tcp_server_info->fd_external, &rfds);
        if (tcp_server_info->fd_external > maxfd) maxfd = tcp_server_info->fd_external;
    }

   
    for (int i = 0; i < tcp_server_info->n_internals; i++) {
        if (tcp_server_info->fd_internals[i] != -1) {
            FD_SET(tcp_server_info->fd_internals[i], &rfds);
            if (tcp_server_info->fd_internals[i] > maxfd) maxfd = tcp_server_info->fd_internals[i];
        }
    }

    int counter = select(maxfd + 1, &rfds, NULL, NULL, &timeout);

    if (counter > 0) {
        if (FD_ISSET(0, &rfds)) {
            return 1; 
        } else if (FD_ISSET(tcp_server_info->fd, &rfds)) {
            return 2; 
        } else if ( FD_ISSET(tcp_server_info->fd_external, &rfds)) {
            return 5; 
        } else {
            for (int i = 0; i < tcp_server_info->n_internals; i++) {
                if (FD_ISSET(tcp_server_info->fd_internals[i], &rfds)) {
                    *internal_index = i;
                    return 4; 
                }
            }
        }
    } else if (counter < 0) {
        perror("Erro na chamada select");
    }

    return -1; 
}


int get_user_input(udp_comms *comms_info_to_node_server, tcp_socket *server_info, cache_info *ci, interest_table *it, int j) {
    char user_input[200] = "";

    if (fgets(user_input, 200, stdin) == NULL){
        perror("Erro ao ler a entrada do usuário");
        exit(EXIT_FAILURE);
    }
    fflush(stdin);

    if (strncmp(user_input, "help", 4) == 0 || strncmp(user_input, "h", 1) == 0) {
        show_help();
        return 1;
    } else if (strncmp(user_input, "join", 4) == 0 || strncmp(user_input, "j", 1) == 0) {
        char net[4];  
        
       
        int n = sscanf(user_input, "%*s %s", net);  
    
        if (n != 1) {
            printf("Erro: Formato de comando inválido. Use 'join <net>' onde <net> é um número de 3 dígitos entre 001 e 999.\n");
            return 0;
        }
    
        if (strlen(net) != 3 || net[0] < '0' || net[0] > '9' || net[1] < '0' || net[1] > '9' || net[2] < '0' || net[2] > '9') {
            printf("Erro: <net> deve ser um número de 3 dígitos entre 001 e 999.\n");
            return 0;
        }
    

        int net_value = atoi(net);
        if (net_value < 1 || net_value > 999) {
            printf("Erro: <net> deve estar entre 001 e 999.\n");
            return 0;
        }
    
        strncpy(server_info->net, net, sizeof(server_info->net));  
        server_info->net[3] = '\0';  
    
        if (join(comms_info_to_node_server, server_info, j) == 0) {
            printf("[ERRO] Falha ao entrar na rede.\n");
            return 0;
        }
        return 1;
    } else if (strncmp(user_input, "direct join", 11) == 0 || strncmp(user_input, "dj", 2) == 0) {
        char connectIP[INET_ADDRSTRLEN] = "";
        char connectPort[6] = "";
        char net[4];
        if (sscanf(user_input, "direct join %s %s %s", net, connectIP, connectPort) != 3 &&
            sscanf(user_input, "dj %s %s %s", net, connectIP, connectPort) != 3) {
            printf("[ERRO] Formato incorreto para o comando direct join.\n");
            return 0;
        }
        if (direct_join(net, connectIP, connectPort, server_info) == 0) {
            printf("[ERRO] Falha ao entrar na rede diretamente.\n");
            return 0;
        }
        return 1;
    } else if (strncmp(user_input, "create", 6) == 0 || strncmp(user_input, "c", 1) == 0) {
        char name[100] = "";
        if (sscanf(user_input, "create %99s", name) != 1 && sscanf(user_input, "c %99s", name) != 1) {
            printf("[ERRO] Formato incorreto para o comando create.\n");
            return 0;
        }
        if (create_object(name, ci) == 0) {
            printf("[ERRO] Falha ao criar objeto.\n");
            return 0;
        }
        return 1;
    } else if (strncmp(user_input, "delete", 6) == 0 || strncmp(user_input, "dl", 2) == 0) {
        char name[100] = "";
        if (sscanf(user_input, "delete %99s", name) != 1 && sscanf(user_input, "dl %99s", name) != 1) {
            printf("[ERRO] Formato incorreto para o comando delete.\n");
            return 0;
        }
        if (delete_object(name, ci) == 0) {
            printf("[ERRO] Falha ao deletar objeto.\n");
            return 0;
        }
        return 1;
    } else if (strncmp(user_input, "retrieve", 8) == 0 || strncmp(user_input, "r", 1) == 0) {
        char name[100] = "";
        if (sscanf(user_input, "retrieve %99s", name) != 1 && sscanf(user_input, "r %99s", name) != 1) {
            printf("[ERRO] Formato incorreto para o comando retrieve.\n");
            return 0;
        }
        if (retrieve_object(name, server_info, ci, it) == 0) {
            printf("[ERRO] Falha ao recuperar objeto.\n");
            return 0;
        }
        return 1;
    } else if (strncmp(user_input, "show names", 10) == 0 || strncmp(user_input, "sn", 2) == 0) {
        show_names(ci);
        return 1;
    } else if (strncmp(user_input, "show interest table", 19) == 0 || strncmp(user_input, "si", 2) == 0) {
        show_interest_table(it);
        return 1;
    } else if (strncmp(user_input, "show topology", 13) == 0 || strncmp(user_input, "st", 2) == 0) {
        show_topology(server_info);
        return 1;
    } else if (strncmp(user_input, "leave", 5) == 0 || strncmp(user_input, "l", 1) == 0) {
        if (leave_network(comms_info_to_node_server, server_info->net, server_info, ci, it) == 0) {
            printf("[ERRO] Falha ao sair da rede.\n");
            return 0;
        }
        return 1;
    } else if (strncmp(user_input, "exit", 4) == 0 || strncmp(user_input, "x", 1) == 0) {
        exit_application(comms_info_to_node_server, server_info->net, server_info, ci, it);
        return 2;
    }

    return 0;
}