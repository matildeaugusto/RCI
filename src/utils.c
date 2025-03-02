/******************************************************************************
 * Nome do Ficheiro: utils.c
 * 
 * Autores:   
 * 
 * Descrição: Funções de utilidade, verificações e inicializações.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <arpa/inet.h>
#include "utils.h"

/******************************************************************************
 * is_string_integer ()
 *
 * Argumentos: 
 * str - A string que será verificada para determinar se representa um inteiro.
 * 
 * Returns: 1 se a string representa um número inteiro, 0 caso contrário.
 * 
 * Side-Effects: Nenhum
 *
 * Descrição: Verifica se uma string representa um número inteiro, percorrendo cada caractere e garantindo que todos são dígitos.
 *****************************************************************************/
int is_string_integer(const char *str) {
  if (str == NULL || *str == '\0') return 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if (!isdigit((unsigned char)str[i])) return 0;
  }
  return 1;
}

/******************************************************************************
 * isValidIpAddress ()
 *
 * Argumentos: 
 * ip - O endereço IP a ser validado.
 * 
 * Returns: 1 se o endereço IP for válido, 0 caso contrário.
 * 
 * Side-Effects: Nenhum
 *
 * Descrição: Valida um endereço IP utilizando a função inet_pton. Se o endereço for válido, a função retorna 1, caso contrário, retorna 0.
 *****************************************************************************/
int isValidIpAddress(const char *ip) {
  struct sockaddr_in sa;
  return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1;
}
