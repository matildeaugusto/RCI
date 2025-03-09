#define _GNU_SOURCE

#include "server.h"

int open_tcp_socket(tcp_socket *server_info) {
    struct addrinfo hints, *res;
    int fd, errcode;

    // Criação do socket
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erro ao criar o socket");
        return 0; // erro
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE;

    // Resolver o endereço
    if ((errcode = getaddrinfo(server_info->ip, server_info->port, &hints, &res)) != 0) {
        fprintf(stderr, "Erro ao resolver o endereço: %s\n", gai_strerror(errcode));
        close(fd);
        return 0;
    }

    server_info->res = res;


    // Bind
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Erro ao fazer bind");
        freeaddrinfo(res);
        close(fd);
        return 0; // erro
    }

    // Listen
    if (listen(fd, 5) == -1) {
        perror("Erro ao escutar por conexões");
        freeaddrinfo(res);
        close(fd);
        return 0; // erro
    }

    server_info->fd = fd;
    printf("Servidor TCP criado com sucesso\n");
    return 1; // sucesso
}

int accept_incoming_connection(tcp_socket *server_info) {
    socklen_t addrlen;
    struct sockaddr_storage addr;
    int newfd;
    char ipstr[INET_ADDRSTRLEN];
    char portstr[6];

    addrlen = sizeof(addr);
    if ((newfd = accept(server_info->fd, (struct sockaddr *)&addr, &addrlen)) == -1) {
        perror("accept");
        return 0;
    }

    // Obter informações sobre o endereço do novo cliente
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)&addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);
    snprintf(portstr, sizeof(portstr), "%d", ntohs(ipv4->sin_port));

    // Determinar se a nova conexão é de um vizinho interno ou externo
    if (server_info->fd_external == -1) {
        // Se não há vizinho externo, a nova conexão é o vizinho externo
        server_info->fd_external = newfd;
        strcpy(server_info->external_ip, ipstr);
        strcpy(server_info->external_port, portstr);
    } else {
        // Nova conexão é de um vizinho interno
        if (server_info->n_internals < 10) {
            server_info->fd_internals[server_info->n_internals] = newfd;
            strcpy(server_info->internals_ip[server_info->n_internals], ipstr);
            strcpy(server_info->internals_port[server_info->n_internals], portstr);
            server_info->n_internals++;
        } else {
            // Tratar o caso em que não há espaço para mais vizinhos internos
            close(newfd);
            printf("Não há espaço para mais vizinhos internos.\n");
            return 0;
        }
    }

    return 1;
}

int receive_tcp_message_from_out(tcp_socket *server_info, cache_info *ci, interest_table *it) {
    char buff[5000] = "";
    char *message = buff;

    // Processa a mensagem recebida
    if (strncmp(message, "INTEREST", 8) == 0) {
        // Trata mensagem de interesse
        char name[100];
        if (sscanf(message, "INTEREST %99s", name) == 1) {
            handle_interest_message(name, server_info, ci, it);
        }
    } else if (strncmp(message, "OBJECT", 6) == 0) {
        // Trata mensagem de objeto
        char name[100];
        if (sscanf(message, "OBJECT %99s", name) == 1) {
            handle_object_message(name, message + 8, server_info, ci, it);
        }
    } else if (strncmp(message, "NOOBJECT", 8) == 0) {
        // Trata mensagem de ausência
        char name[100];
        if (sscanf(message, "NOOBJECT %99s", name) == 1) {
            handle_no_object_message(name, server_info, ci, it);
        }
    } else if (strncmp(message, "ENTRY", 4) == 0) {
        // Trata mensagem de entrada
        char new_node_ip[INET_ADDRSTRLEN];
        char new_node_port[6];
        if (sscanf(message, "JOIN %s %5s", new_node_ip, new_node_port) == 2) {
           // handle_join_message(new_node_ip, new_node_port, server_info);
        }
    }  else if (strncmp(message, "BACKUP", 6) == 0) {
        char backup_ip[INET_ADDRSTRLEN];
        char backup_port[6];
        if (sscanf(message, "BACKUP %s %5s", backup_ip, backup_port) == 2) {
            handle_backup_message(backup_ip, backup_port, server_info);
        }
    } else {
        // Trata outros tipos de mensagens, se necessário
        printf("Mensagem desconhecida recebida: %s\n", message);
    }

    return 1;
}

int receive_tcp_message_from_internal(tcp_socket *server_info, cache_info *ci, interest_table *it, int internal_index) {
    char buff[5000] = "";
    char *message = buff;

    // Recebe a mensagem do socket interno
    int bytes_received = recv(server_info->fd_internals[internal_index], buff, sizeof(buff) - 1, 0);
    if (bytes_received <= 0) {
        perror("Erro ao receber mensagem do vizinho interno");
        return 0;
    }
    buff[bytes_received] = '\0';  // Garante que a string seja terminada corretamente

    printf("%s\n", message);  // Log da mensagem recebida

    // Processa a mensagem recebida
    if (strncmp(message, "INTEREST", 8) == 0) {
        // Trata mensagem de interesse
        char name[100];
        if (sscanf(message, "INTEREST %99s", name) == 1) {
            handle_interest_message(name, server_info, ci, it);
        }
    } else if (strncmp(message, "OBJECT", 6) == 0) {
        // Trata mensagem de objeto
        char name[100];
        if (sscanf(message, "OBJECT %99s", name) == 1) {
            handle_object_message(name, message + 8, server_info, ci, it);
        }
    } else if (strncmp(message, "NOOBJECT", 8) == 0) {
        // Trata mensagem de ausência
        char name[100];
        if (sscanf(message, "NOOBJECT %99s", name) == 1) {
            handle_no_object_message(name, server_info, ci, it);
        }
    } else if (strncmp(message, "ENTRY", 5) == 0) {
        // Trata mensagem de entrada
        char new_node_ip[INET_ADDRSTRLEN];
        char new_node_port[6];
        if (sscanf(message, "ENTRY %s %5s", new_node_ip, new_node_port) == 2) {
            handle_join_message(new_node_ip, new_node_port, server_info, internal_index);
        } else {
            printf("[DEBUG] Falha ao analisar a mensagem ENTRY.\n");
        }
    } else {
        // Trata outros tipos de mensagens, se necessário
        printf("Mensagem desconhecida recebida do vizinho interno: %s\n", message);
    }

    return 1;
}

int receive_tcp_message_from_external(tcp_socket *server_info, cache_info *ci, interest_table *it, udp_comms *node_server_info, int j, int internal_index) {
    char buff[5000] = "";
    char *message = buff;

    // Recebe a mensagem do socket externo
    int bytes_received = recv(server_info->fd_external, buff, sizeof(buff) - 1, 0);
    if (bytes_received <= 0) {
        // perror("Erro ao receber mensagem do vizinho externo");
        return 0;
    }
    buff[bytes_received] = '\0';  // Garante que a string seja terminada corretamente

    printf("%s\n", message);  // Log da mensagem recebida

    // Processa a mensagem recebida
    if (strncmp(message, "INTEREST", 8) == 0) {
        // Trata mensagem de interesse
        char name[100];
        if (sscanf(message, "INTEREST %99s", name) == 1) {
            handle_interest_message(name, server_info, ci, it);
        }
    } else if (strncmp(message, "OBJECT", 6) == 0) {
        // Trata mensagem de objeto
        char name[100];
        if (sscanf(message, "OBJECT %99s", name) == 1) {
            handle_object_message(name, message + 8, server_info, ci, it);
        }
    } else if (strncmp(message, "NOOBJECT", 8) == 0) {
        // Trata mensagem de ausência
        char name[100];
        if (sscanf(message, "NOOBJECT %99s", name) == 1) {
            handle_no_object_message(name, server_info, ci, it);
        }
    } else if (strncmp(message, "ENTRY", 5) == 0) {
        // Trata mensagem de entrada
        char new_node_ip[INET_ADDRSTRLEN];
        char new_node_port[6];
        if (sscanf(message, "ENTRY %s %5s", new_node_ip, new_node_port) == 2) {
            handle_join_message_j(new_node_ip, new_node_port, server_info, internal_index);
        }
    } else if (strncmp(message, "SAFE", 4) == 0 ) {
        // Trata mensagem SAFE
        char sender_ip[INET_ADDRSTRLEN];
        char sender_port[6];
        if (sscanf(message, "SAFE %s %s", sender_ip, sender_port) == 2) {
            // Envia a mensagem REG para o servidor de nós
            // send_reg_message(node_server_info, server_info->ip, server_info->port, server_info->net);
        }
    } else {
        // Trata outros tipos de mensagens, se necessário
        printf("Mensagem desconhecida recebida do vizinho externo: %s\n", message);
    }

    return 1;
}

void handle_interest_message(char *name, tcp_socket *server_info, cache_info *ci, interest_table *it) {
    // Verifica se o objeto está no cache
    for (int i = 0; i < ci->size; i++) {
        if (strcmp(ci->entries[i].name, name) == 0) {
            // Objeto encontrado no cache, envia mensagem de objeto
            send_object_message(name, ci->entries[i].data, server_info);
            return;
        }
    }

    // Verifica se o objeto já tem uma entrada na tabela de interesses pendentes
    int entry_index = -1;
    for (int i = 0; i < it->size; i++) {
        if (strcmp(it->entries[i].name, name) == 0) {
            entry_index = i;
            break;
        }
    }

    if (entry_index == -1) {
        // Adiciona uma nova entrada na tabela de interesses pendentes
        if (it->size < MAX_SIZE) {
            strcpy(it->entries[it->size].name, name);
            it->entries[it->size].interface_count = 0;
            for (int j = 0; j < 10; j++) {
                it->entries[it->size].state[j] = WAITING;
            }
            it->size++;
            entry_index = it->size - 1;
        } else {
            printf("Tabela de interesses pendentes cheia.\n");
            return;
        }
    }

    // Reencaminha a mensagem de interesse para os vizinhos
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            send_interest_message(name, server_info->fd_internals[i]);
        }
    }
    if (server_info->fd_external != -1) {
        send_interest_message(name, server_info->fd_external);
    }

    // Atualiza o estado da interface na tabela de interesses pendentes
    it->entries[entry_index].state[it->entries[entry_index].interface_count] = WAITING;
    it->entries[entry_index].interface_count++;
}

void handle_object_message(char *name, char *data, tcp_socket *server_info, cache_info *ci, interest_table *it) {
    // Verifica se o objeto está na tabela de interesses pendentes
    for (int i = 0; i < it->size; i++) {
        if (strcmp(it->entries[i].name, name) == 0) {
            // Reencaminha a mensagem de objeto para todas as interfaces no estado de resposta
            for (int j = 0; j < 10; j++) {
                if (it->entries[i].state[j] == RESPONDED) {
                    send_object_message(name, data, server_info);
                }
            }
            // Remove a entrada da tabela de interesses pendentes
            for (int k = i; k < it->size - 1; k++) {
                it->entries[k] = it->entries[k + 1];
            }
            it->size--;
            break;
        }
    }

    // Armazena o objeto no cache
    if (ci->size < MAX_SIZE) {
        strcpy(ci->entries[ci->size].name, name);
        strcpy(ci->entries[ci->size].data, data);
        ci->entries[ci->size].timestamp = time(NULL);
        ci->size++;
    } else {
        printf("Cache cheio, não foi possível armazenar o objeto.\n");
    }

    // Reencaminha a mensagem de objeto para os vizinhos
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            send_object_message(name, data, server_info);
        }
    }
    if (server_info->fd_external != -1) {
        send_object_message(name, data, server_info);
    }
}

void handle_no_object_message(char *name, tcp_socket *server_info, cache_info *ci, interest_table *it) {
    // Verifica se o objeto está na tabela de interesses pendentes
    for (int i = 0; i < it->size; i++) {
        if (strcmp(it->entries[i].name, name) == 0) {
            // Atualiza o estado da interface para fechado
            for (int j = 0; j < 10; j++) {
                if (it->entries[i].state[j] == WAITING) {
                    it->entries[i].state[j] = CLOSED;
                }
            }

            // Verifica se todas as interfaces estão fechadas
            int all_closed = 1;
            for (int j = 0; j < 10; j++) {
                if (it->entries[i].state[j] != CLOSED) {
                    all_closed = 0;
                    break;
                }
            }

            // Se todas as interfaces estiverem fechadas, reencaminha a mensagem de ausência
            if (all_closed) {
                for (int j = 0; j < 10; j++) {
                    if (it->entries[i].state[j] == RESPONDED) {
                        send_no_object_message(name, server_info);
                    }
                }
            }

            // Remove a entrada da tabela de interesses pendentes
            for (int k = i; k < it->size - 1; k++) {
                it->entries[k] = it->entries[k + 1];
            }
            it->size--;
            break;
        }
    }

    // Reencaminha a mensagem de ausência para os vizinhos
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            send_no_object_message(name, server_info);
        }
    }
    if (server_info->fd_external != -1) {
        send_no_object_message(name, server_info);
    }
}

void send_object_message(char *name, char *data, tcp_socket *server_info) {
    char message[1024];
    snprintf(message, sizeof(message), "OBJECT %s %s", name, data);

    // Envia a mensagem para o vizinho externo, se existir
    if (server_info->fd_external != -1) {
        if (write(server_info->fd_external, message, strlen(message)) == -1) {
            perror("Erro ao enviar mensagem de objeto para o vizinho externo");
        }
    }

    // Envia a mensagem para os vizinhos internos
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            if (write(server_info->fd_internals[i], message, strlen(message)) == -1) {
                perror("Erro ao enviar mensagem de objeto para o vizinho interno");
            }
        }
    }
}

void send_interest_message(char *name, int fd) {
    char message[1024];
    snprintf(message, sizeof(message), "INTEREST %s", name);

    if (write(fd, message, strlen(message)) == -1) {
        perror("Erro ao enviar mensagem de interesse");
    }
}

void send_no_object_message(char *name, tcp_socket *server_info) {
    char message[1024];
    snprintf(message, sizeof(message), "NOOBJECT %s", name);

    // Envia a mensagem para o vizinho externo, se existir
    if (server_info->fd_external != -1) {
        if (write(server_info->fd_external, message, strlen(message)) == -1) {
            perror("Erro ao enviar mensagem de ausência para o vizinho externo");
        }
    }

    // Envia a mensagem para os vizinhos internos
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            if (write(server_info->fd_internals[i], message, strlen(message)) == -1) {
                perror("Erro ao enviar mensagem de ausência para o vizinho interno");
            }
        }
    }
}

int create_object(char *name, cache_info *ci) {
    // Verifica se o cache está cheio
    if (ci->size >= MAX_SIZE) {
        printf("Erro: Cache está cheio, não foi possível criar o objeto.\n");
        return 0;
    }

    // Adiciona o objeto ao cache
    strcpy(ci->entries[ci->size].name, name);
    ci->entries[ci->size].data[0] = '\0'; // Não estamos armazenando dados, então deixamos vazio
    ci->entries[ci->size].timestamp = time(NULL);
    ci->size++;

    printf("Objeto '%s' criado com sucesso.\n", name);
    return 1;
}

int delete_object(char *name, cache_info *ci) {
    // Procura o objeto no cache
    for (int i = 0; i < ci->size; i++) {
        if (strcmp(ci->entries[i].name, name) == 0) {
            // Remove o objeto do cache
            for (int j = i; j < ci->size - 1; j++) {
                ci->entries[j] = ci->entries[j + 1];
            }
            ci->size--;
            printf("Objeto '%s' removido com sucesso.\n", name);
            return 1;
        }
    }

    // Se o objeto não for encontrado
    printf("Erro: Objeto '%s' não encontrado no cache.\n", name);
    return 0;
}

int retrieve_object(char *name, tcp_socket *server_info, cache_info *ci, interest_table *it) {
    // Verifica se o objeto está no cache
    for (int i = 0; i < ci->size; i++) {
        if (strcmp(ci->entries[i].name, name) == 0) {
            printf("Objeto '%s' encontrado no cache.\n", name);
            // Aqui você pode retornar ou exibir os dados do objeto, se necessário
            return 1;
        }
    }

    // Se o objeto não estiver no cache, envia uma mensagem de interesse
    send_interest_message(name, server_info->fd_external);
    for (int i = 0; i < server_info->n_internals; i++) {
        send_interest_message(name, server_info->fd_internals[i]);
    }

    // Atualiza a tabela de interesses pendentes
    if (it->size < MAX_SIZE) {
        strcpy(it->entries[it->size].name, name);
        it->entries[it->size].interface_count = 0;
        for (int j = 0; j < 10; j++) {
            it->entries[it->size].state[j] = WAITING;
        }
        it->size++;
    } else {
        printf("Tabela de interesses pendentes cheia.\n");
    }

    printf("Mensagem de interesse enviada para o objeto '%s'.\n", name);
    return 1;
}

void show_names(cache_info *ci) {
    if (ci->size == 0) {
        printf("Não há objetos armazenados no cache.\n");
        return;
    }

    printf("Objetos armazenados no cache:\n");
    for (int i = 0; i < ci->size; i++) {
        printf("%s\n", ci->entries[i].name);
    }
}

void show_interest_table(interest_table *it) {
    if (it->size == 0) {
        printf("A tabela de interesses pendentes está vazia.\n");
        return;
    }

    printf("Tabela de Interesses Pendentes:\n");
    for (int i = 0; i < it->size; i++) {
        printf("Objeto: %s\n", it->entries[i].name);
        printf("Estados das Interfaces:\n");
        for (int j = 0; j < it->entries[i].interface_count; j++) {
            switch (it->entries[i].state[j]) {
                case WAITING:
                    printf("Interface %d: Espera\n", j);
                    break;
                case RESPONDED:
                    printf("Interface %d: Respondido\n", j);
                    break;
                case CLOSED:
                    printf("Interface %d: Fechado\n", j);
                    break;
                default:
                    printf("Interface %d: Estado desconhecido\n", j);
                    break;
            }
        }
        printf("\n");
    }
}

int leave_network(udp_comms *comms_info_to_node_server, char *net, tcp_socket *server_info, cache_info *ci, interest_table *it) {
   
    // send_unreg_message(comms_info_to_node_server, net, server_info->ip, server_info->port);
    // Envia uma mensagem ao servidor de nós para remover o registro do nó
    char message[100];
    snprintf(message, sizeof(message), "UNREG %s %s %s", net, server_info->ip, server_info->port);
    if (sendto(comms_info_to_node_server->fd, message, strlen(message), 0, comms_info_to_node_server->res->ai_addr, comms_info_to_node_server->res->ai_addrlen) == -1) {
        perror("Erro ao enviar mensagem de desregistro ao servidor de nós");
        return 0;
    }

    // Fecha todas as conexões TCP
    if (server_info->fd_external != -1) {
        close(server_info->fd_external);
        server_info->fd_external = -1;
    }
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            close(server_info->fd_internals[i]);
            server_info->fd_internals[i] = -1;
        }
    }

    // Fecha o socket TCP do servidor
    if (server_info->fd != -1) {
        close(server_info->fd);
        server_info->fd = -1;
    }

    // Libera recursos alocados
    freeaddrinfo(comms_info_to_node_server->res);
    freeaddrinfo(server_info->res);
    free(ci);
    free(it);

    printf("Nó saiu da rede com sucesso.\n");
    return 1;
}

void exit_application(udp_comms *comms_info_to_node_server, char *net, tcp_socket *server_info, cache_info *ci, interest_table *it) {
    // Fecha todas as conexões TCP
    if (server_info->fd_external != -1) {
        close(server_info->fd_external);
    }
    for (int i = 0; i < server_info->n_internals; i++) {
        if (server_info->fd_internals[i] != -1) {
            close(server_info->fd_internals[i]);
        }
    }

    // Fecha o socket TCP do servidor
    if (server_info->fd != -1) {
        close(server_info->fd);
    }

    // Libera recursos alocados
    if (comms_info_to_node_server->res != NULL) {
        freeaddrinfo(comms_info_to_node_server->res);
    }
    if (server_info->res != NULL) {
        freeaddrinfo(server_info->res);
    }
    if (ci != NULL) {
        free(ci);
    }
    if (it != NULL) {
        free(it);
    }

    printf("Aplicação encerrada.\n");

    // Encerra o programa
    exit(EXIT_SUCCESS);
}

int join(udp_comms *comms_info_to_node_server, tcp_socket *server_info, int j) {
    char message[4096];  // Aumentar o tamanho do buffer 'message'
    char response[8192]; // Aumentar o tamanho do buffer 'response'
    char ip[INET_ADDRSTRLEN];
    char port[6];
    char *line;
    char *nodes[MAX_SIZE];  // Supondo que há no máximo 100 nós
    int node_count = 0;
    j = 1;
    
    // Solicita a lista de nós ao servidor de nós
    snprintf(message, sizeof(message), "NODES %s", server_info->net);

    if (sendto(comms_info_to_node_server->fd, message, strlen(message), 0, comms_info_to_node_server->res->ai_addr, comms_info_to_node_server->res->ai_addrlen) == -1) {
        perror("Erro ao enviar mensagem NODES");
        return 0;
    }

    // Recebe a resposta do servidor de nós
    socklen_t addrlen = comms_info_to_node_server->res->ai_addrlen;
    ssize_t len = recvfrom(comms_info_to_node_server->fd, response, sizeof(response) - 1, 0, comms_info_to_node_server->res->ai_addr, &addrlen);
    if (len <= 0) {
        perror("Erro ao receber resposta do servidor de nós");
        return 0;
    }

    // Adiciona o caractere de terminação de string
    response[len] = '\0';
    printf("%s\n", response);  

    // Processa a resposta
    line = strtok(response, "\n");

    // Verifica se a primeira linha é NODESLIST <net>
    if (line == NULL || strncmp(line, "NODESLIST", 9) != 0) {
        printf("Erro: Resposta do servidor de nós inválida.\n");
        return 0;
    }

    // Ignora a linha NODESLIST <net>
    line = strtok(NULL, "\n");

    while (line != NULL) {
        if (node_count < MAX_SIZE) {
            // Copia a linha para um buffer seguro
            nodes[node_count] = strdup(line);  // Usar strdup para alocar memória
            if (nodes[node_count] == NULL) {
                perror("Erro ao alocar memória para o nó");
                return 0;
            }
            node_count++;
        }
        line = strtok(NULL, "\n");   // Avança para a próxima linha
    }

    if (node_count == 0) {
        // Se não houver nós na lista, registra o nó como o primeiro nó da rede
        send_reg_message(comms_info_to_node_server, server_info->ip, server_info->port, server_info->net);
        
        // Atualiza a estrutura do nó para refletir que é o primeiro nó
        server_info->fd_external = -1;  // Não há vizinho externo
        strcpy(server_info->external_ip, "");
        strcpy(server_info->external_port, "");
        strcpy(server_info->backup_ip, "");
        strcpy(server_info->backup_port, "");
        server_info->n_internals = 0;  // Não há vizinhos internos

        return 1;
    } else if (node_count > 0) {
        srand(time(NULL));

        int random_node = rand() % node_count;
        line = nodes[random_node];

        if (sscanf(line, "%s %s", ip, port) == 2) {

            if (create_tcp_connection(ip, port, server_info) == 0) {
                printf("Erro ao estabelecer conexão TCP com o nó %s:%s.\n", ip, port);
                return 0;
            }
            send_join_message(server_info->ip, server_info->port, server_info);  


            if (receive_safe_message(server_info->fd_external)) {
                snprintf(message, sizeof(message), "REG %s %s %s", server_info->net, server_info->ip, server_info->port);
                if (sendto(comms_info_to_node_server->fd, message, strlen(message), 0, comms_info_to_node_server->res->ai_addr, comms_info_to_node_server->res->ai_addrlen) == -1) {
                    perror("Erro ao enviar mensagem REG");
                    return 0;
                }

                receive_regok_message(comms_info_to_node_server);
            }

            
            // Registra o nó no servidor de nós
            
            
        }

        // Libera a memória alocada para os nós
        for (int i = 0; i < node_count; i++) {
            free(nodes[i]);
        }        
          
    }

    // Limpeza dos buffers após o uso
    memset(message, 0, sizeof(message));
    memset(response, 0, sizeof(response));  

    return 1;
}

int receive_safe_message(int fd) {
    char buff[5000] = "";
    int bytes_received = recv(fd, buff, sizeof(buff) - 1, 0);

    if (bytes_received <= 0) {
        perror("Erro ao receber mensagem SAFE");
        return 0;
    }
    buff[bytes_received] = '\0';  // Garante que a string seja terminada corretamente

    printf("%s\n", buff);

    // Processa a mensagem recebida
    if (strncmp(buff, "SAFE", 4) == 0) {
        char sender_ip[INET_ADDRSTRLEN];
        char sender_port[6];
        if (sscanf(buff, "SAFE %s %s", sender_ip, sender_port) == 2) {
            return 1; // Indica que a mensagem SAFE foi recebida com sucesso
        }
    } else {
        printf("[DEBUG] Mensagem desconhecida ou erro recebida: %s\n", buff);
    }

    return 0; // Indica que a mensagem SAFE não foi recebida
}

int direct_join(char *net,char *connectIP, char *connectPort, tcp_socket *server_info) {

        // Estabelece conexão TCP diretamente com o nó especificado
    if (create_tcp_connection(connectIP, connectPort, server_info) == 0) {
        printf("Erro ao estabelecer conexão TCP direta com o nó.\n");
        return 0;
    }

    // Envia mensagem ENTRY para informar a entrada do novo nó
    send_join_message(server_info->ip, server_info->port, server_info);

    return 1;
}

int create_tcp_connection(char *ip, char *port, tcp_socket *server_info) {
    struct addrinfo hints, *res;
    int sockfd;
    int errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Tenta obter informações de endereço
    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "Erro ao obter informações de endereço: %s\n", gai_strerror(errcode));
        return 0;
    }

    // Cria o socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        freeaddrinfo(res);
        perror("Erro ao criar socket");
        return 0;
    }

    // Tenta conectar ao endereço
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        close(sockfd);
        perror("Erro ao conectar ao nó");
        return 0;
    }

    freeaddrinfo(res);

    // Atualiza a estrutura tcp_socket com o novo socket e informações do nó externo
    server_info->fd_external = sockfd;
    strncpy(server_info->external_ip, ip, INET_ADDRSTRLEN - 1);
    server_info->external_ip[INET_ADDRSTRLEN - 1] = '\0';
    strncpy(server_info->external_port, port, 5);
    server_info->external_port[5] = '\0';

    return 1;
}

void show_topology(tcp_socket *server_info) {
    printf("Topologia da Rede:\n");
    printf("Vizinho Externo: %s:%s\n", server_info->external_ip, server_info->external_port);
    printf("Nó de Salvaguarda: %s:%s\n", server_info->backup_ip, server_info->backup_port);
    printf("Vizinhos Internos:\n");
    for (int i = 1; i < server_info->n_internals; i++) {
        printf("Vizinho Interno %d: %s:%s\n", i, server_info->internals_ip[i], server_info->internals_port[i]);
    }
}

void send_join_message(char *new_node_ip, char *new_node_port, tcp_socket *server_info) {
    char message[2048];  // Aumentar o tamanho do buffer 'message'

    snprintf(message, sizeof(message), "ENTRY %s %s", server_info->ip, server_info->port);


    // Verifica se o socket externo está aberto
    if (server_info->fd_external != -1) {
        // Imprime o IP e a porta do nó externo
        
        if (write(server_info->fd_external, message, strlen(message)) == -1) {
        } 
    } else {
        printf("[DEBUG] Socket externo não está aberto.\n");
    }

    memset(message, 0, sizeof(message));
}

void handle_join_message_j(char *new_node_ip, char *new_node_port, tcp_socket *server_info, int internal_index) {
    if (server_info->n_internals < 10) {
        server_info->fd_internals[server_info->n_internals] = -1;


        // Garantir espaço para o caractere nulo de terminação
        snprintf(server_info->internals_ip[server_info->n_internals], sizeof(server_info->internals_ip[0]), "%s", new_node_ip);
        snprintf(server_info->internals_port[server_info->n_internals], sizeof(server_info->internals_port[0]), "%s", new_node_port);

        server_info->n_internals++;

        send_safe_message_j(server_info, internal_index);

    } else {
        printf("Não há espaço para mais vizinhos internos.\n");
    }
}

void send_safe_message_j(tcp_socket *server_info, int i) {
    char message[1024];

    snprintf(message, sizeof(message), "SAFE %s %s", server_info->ip, server_info->port);

    if (server_info->fd_external != -1) {
        // Imprime o IP e a porta do nó externo
        
        if (write(server_info->fd_external, message, strlen(message)) == -1) {
            
        } 
    } else {
        printf("[DEBUG] Socket externo não está aberto.\n");
    }
}

void handle_join_message(char *new_node_ip, char *new_node_port, tcp_socket *server_info, int internal_index) {
    if (server_info->n_internals < 10) {
        server_info->fd_internals[server_info->n_internals] = -1;


        // Garantir espaço para o caractere nulo de terminação
        snprintf(server_info->internals_ip[server_info->n_internals], sizeof(server_info->internals_ip[0]), "%s", new_node_ip);
        snprintf(server_info->internals_port[server_info->n_internals], sizeof(server_info->internals_port[0]), "%s", new_node_port);

        server_info->n_internals++;

        send_safe_message(server_info, internal_index);

    } else {
        printf("Não há espaço para mais vizinhos internos.\n");
    }
}

void send_safe_message(tcp_socket *server_info, int i) {
    char message[1024];



    snprintf(message, sizeof(message), "SAFE %s %s", server_info->ip, server_info->port);


    if (server_info->fd_internals[i] != -1) {
        // Imprime o IP e a porta do nó externo
        
        if (write(server_info->fd_internals[i], message, strlen(message)) == -1) {
        } 
    } else {
        printf("[DEBUG] Socket externo não está aberto.\n");
    }
}

void handle_backup_message(char *backup_ip, char *backup_port, tcp_socket *server_info) {
    // Atualiza o nó de salvaguarda
    strcpy(server_info->backup_ip, backup_ip);
    strcpy(server_info->backup_port, backup_port);
    printf("Nó de salvaguarda atualizado: %s:%s\n", backup_ip, backup_port);
}

void send_reg_message(udp_comms *comms_info_to_node_server, char *ip, char *port, char *net) {
    char message[1024];

    // Monta a mensagem REG
    snprintf(message, sizeof(message), "REG %s %s %s", net, ip, port);

    // Envia a mensagem ao servidor de nós
    if (sendto(comms_info_to_node_server->fd, message, strlen(message), 0, comms_info_to_node_server->res->ai_addr, comms_info_to_node_server->res->ai_addrlen) == -1) {
        perror("Erro ao enviar mensagem REG");
    } 

    // Recebe a resposta do servidor de nós
    receive_regok_message(comms_info_to_node_server);
}

/* void send_unreg_message(udp_comms *comms_info_to_node_server, char *net, char *ip, char *port) {
    char message[1024];
    snprintf(message, sizeof(message), "UNREG %s %s %s", net, ip, port);

    // Envia a mensagem de desregistro ao servidor de nós
    if (sendto(comms_info_to_node_server->fd, message, strlen(message), 0, comms_info_to_node_server->res->ai_addr, comms_info_to_node_server->res->ai_addrlen) == -1) {
        perror("Erro ao enviar mensagem UNREG");
    }
}*/

void receive_regok_message(udp_comms *node_server_info) {
    char buff[1024];
    socklen_t addrlen = node_server_info->res->ai_addrlen;
    int bytes_received = recvfrom(node_server_info->fd, buff, sizeof(buff) - 1, 0, node_server_info->res->ai_addr, &addrlen);

    if (bytes_received <= 0) {
        perror("Erro ao receber mensagem do servidor de nós");
        return;
    }
    buff[bytes_received] = '\0';  // Garante que a string seja terminada corretamente

    printf("%s\n", buff);

    // Processa a mensagem recebida
    if (strncmp(buff, "OKREG", 5) == 0) {
        char net[4], ip[16], port[6];
        if (sscanf(buff, "OKREG %3s %15s %5s", net, ip, port) == 3) {
            // printf("[DEBUG] Nó registrado com sucesso: %s %s %s\n", net, ip, port);
            // Aqui você pode adicionar qualquer lógica adicional necessária após o registro bem-sucedido
        }
    } else {
        printf("[DEBUG] Mensagem desconhecida ou erro recebida do servidor de nós: %s\n", buff);
    }
}

