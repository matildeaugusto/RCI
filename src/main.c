/*==============================================================*/
/* Programa: Redes de Dados Identificados por Nome              */
/*                                                              */
/* Descrição: Implementaçao de uma rede de dados identificados  */
/* por nome (NDN), um modelo distribuido em que objetos de      */
/* dados sao recuperados diretamente pelo seu nome, sem         */
/* depender de indereços IP ou servidores centrais. As          */
/* mensagens de interesse sao enviadas pelos nos para solicitar */
/* objetos, que sao retornados pelos nos que os possuem,        */
/* armazenando-os em cache ao longo do caminho. A NDN deste     */
/* projeto sera uma rede de sobreposicao baseada na internet,   */
/* utilizando conexoes TCP entre nos e adorando uma topologia   */
/* em arvores para simplificar a gestao da rede.                */
/*                                                              */
/* Ficheiros: (1) main.c                                        */
/*            (2) ndn_struct.c                                  */
/*            (3) utils.c                                       */
/*                                                              */
/* Autores: Matilde Augusto - n.º 103204                        */        
/*          ----- ------ - n.º ------                           */
/*                                                              */
/*==============================================================*/

#include <stdio.h>
#include "ndn_struct.h"
#include "tcp_server.h"

int main(int argc, char *argv[]) {
  NdnConfig config = parse_arguments(argc, argv);

  //PARA RETIRAR
  printf("Cache Size: %d\n", config.cache_size);
  printf("IP: %s\n", config.ip);
  printf("TCP Port: %d\n", config.tcp_port);
  printf("Reg IP: %s\n", config.reg_ip);
  printf("Reg UDP Port: %d\n", config.reg_udp);

  tcp_server(&config);
  return 0;
}
