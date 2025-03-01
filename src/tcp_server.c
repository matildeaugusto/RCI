/******************************************************************************
 *
 * Nome do Ficheiro: tcp_server.c
 * Autores: --------------------------  
 * 
 * DESCRICAO
 * Este ficheiro contém a implementação da função start_tcp_server, que inicia um
 * servidor TCP para comunicação em uma rede NDN. O servidor escuta conexões na
 * porta definida em NdnConfig, aceita conexões de clientes, recebe e envia dados
 * de volta, funciona como um servidor de eco.
 *
 *****************************************************************************/

#define _GNU_SOURCE  

#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void start_tcp_server(NdnConfig *config) {
    int server_fd, client_fd;
    struct addrinfo hints, *res;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];
    ssize_t bytes_read;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("ERROR: Failed to create socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("ERROR: setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", config->tcp_port);

    if (getaddrinfo(NULL, port_str, &hints, &res) != 0) {
        perror("ERROR: getaddrinfo failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("ERROR: bind failed");
        close(server_fd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("ERROR: listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", config->tcp_port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("ERROR: accept failed");
            continue;  
        }

        printf("Client connected\n");

        while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';  
            printf("Received: %s\n", buffer);

            ssize_t bytes_sent = 0;
            while (bytes_sent < bytes_read) {
                ssize_t n = write(client_fd, buffer + bytes_sent, bytes_read - bytes_sent);
                if (n == -1) {
                    perror("ERROR: write failed");
                    break;
                }
                bytes_sent += n;
            }
        }

        if (bytes_read == 0) {
            printf("Client disconnected\n");
        } else if (bytes_read == -1) {
            perror("ERROR: read failed");
        }

        close(client_fd);
    }

    close(server_fd);
}