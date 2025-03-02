/******************************************************************************
 *
 * Nome do Ficheiro: ndn_struct.c
 * Autores:-----------------
 * 
 * DESCRICAO
 * Este ficheiro contém a implementação da função parse_arguments, que processa
 * os argumentos da linha de comando para configurar os parâmetros de uma rede NDN.
 * Ele valida os argumentos fornecidos, como tamanho da cache, endereços IP e portas,
 * garantindo que os valores inseridos sejam corretos antes de armazená-los na
 * estrutura NdnConfig.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ndn_struct.h"
#include "utils.h"

NdnConfig parse_arguments(int argc, char *argv[]) {
  NdnConfig config;

  if (argc < 4 || argc > 6) {
    printf("ERROR: Incorrect program invocation. Usage: ./ndn cache IP TCP [regIP] [regUDP]\n");
    exit(EXIT_FAILURE);
  }

  if (!is_string_integer(argv[1])) {
    printf("ERROR: Invalid cache size. Must be an integer.\n");
    exit(EXIT_FAILURE);
  }
  config.cache_size = atoi(argv[1]);

  if (!isValidIpAddress(argv[2])) {
    printf("ERROR: Invalid IP address.\n");
    exit(EXIT_FAILURE);
  }
  strcpy(config.ip, argv[2]);

  if (!is_string_integer(argv[3])) {
    printf("ERROR: Invalid TCP port. Must be an integer.\n");
    exit(EXIT_FAILURE);
  }
  config.tcp_port = atoi(argv[3]);

  if (argc >= 5) {
    if (!isValidIpAddress(argv[4])) {
      printf("ERROR: Invalid regIP address.\n");
      exit(EXIT_FAILURE);
    }
    strcpy(config.reg_ip, argv[4]);
  } else {
    strcpy(config.reg_ip, DEFAULT_REG_IP);
  }

  if (argc == 6) {
    if (!is_string_integer(argv[5])) {
      printf("ERROR: Invalid regUDP port. Must be an integer.\n");
      exit(EXIT_FAILURE);
    }
    config.reg_udp = atoi(argv[5]);
  } else {
    config.reg_udp = atoi(DEFAULT_REG_UDP);
  }

  return config;
}
