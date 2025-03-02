/******************************************************************************
 *
 * Nome do Ficheiro: tcp_server.c
 * Autores: --------------------------  
 * 
 * DESCRICAO
 * Este ficheiro contém a implementação da função tcp_server, que inicia um
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

void tcp_server(NdnConfig *config) {
  int fd, newfd, n;
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  char buffer[1024];
  ssize_t bytes_read;

  fd = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
  if(fd==-1)exit(1); //error 

  // Socket Configuration
  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    perror("ERROR: setsockopt failed");
    close(fd);
    exit(EXIT_FAILURE);
  }

  // Address Configuration
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP socket
  hints.ai_flags = AI_PASSIVE;

  // Obtaining Address Information
  char port_str[6];
  snprintf(port_str, sizeof(port_str), "%d", config->tcp_port);

  n = getaddrinfo(NULL, port_str, &hints, &res);
  if (n != 0) /*error*/ exit(1);

  // Socket Binding
  n = bind(fd, res->ai_addr, res->ai_addrlen);
  if (n == -1) /*error*/ exit(1);

  // Listening for Connections
  if (listen(fd, SOMAXCONN) == -1) /*error*/ exit(1);

  //PARA RETIRAR - TALVEZ
  printf("Server listening on port %d\n", config->tcp_port);

  while (1) {

    addrlen = sizeof(addr);
    if ((newfd = accept(fd, (struct sockaddr *)&addr, &addrlen)) == -1) /*error*/ exit(1);
    
    // PARA RETIRAR
    printf("Client connected\n");

    // Server Main Loop - just echo for now
    while ((bytes_read = read(newfd, buffer, sizeof(buffer) - 1)) > 0) {
      if (bytes_read == -1) /*error*/ exit(1);

      buffer[bytes_read] = '\0';  
      
      write(1,"received: ",10);
      write(1,buffer,bytes_read);

      ssize_t bytes_sent = 0;
      while (bytes_sent < bytes_read) {
        ssize_t n = write(newfd, buffer + bytes_sent, bytes_read - bytes_sent);
        if (n == -1) /*error*/ exit(1);
        bytes_sent += n;
      }
    }

    // Client Disconnection
    if (bytes_read == 0) {
      printf("Client disconnected\n");
    } else if (bytes_read == -1) {
      perror("ERROR: read failed");
    }

    close(newfd);
  }
  freeaddrinfo(res);
  close(fd);
}