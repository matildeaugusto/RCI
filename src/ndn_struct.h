/******************************************************************************
 *
 * Nome do Ficheiro: ndn_struct.h
 * Autores: -----------------------   
 * 
 * Descrição:
 * Este ficheiro contém a definição da estrutura `NdnConfig`, que armazena os 
 * parâmetros de configuração do programa, bem como a declaração da função 
 * `parse_arguments()` para processar os argumentos da linha de comando.
 * 
 *****************************************************************************/

 #ifndef NDN_STRUCT_H
 #define NDN_STRUCT_H
 
 #include <stdio.h>
 
 #define MAX_IP_LENGTH 16
 #define MAX_PORT_LENGTH 6
 #define DEFAULT_REG_IP "193.136.138.142"
 #define DEFAULT_REG_UDP "59000"
 
 typedef struct {
     int cache_size;              
     char ip[MAX_IP_LENGTH];      
     int tcp_port;                
     char reg_ip[MAX_IP_LENGTH];  
     int reg_udp;                 
 } NdnConfig;
 
 NdnConfig parse_arguments(int argc, char *argv[]);
 
 #endif /* NDN_STRUCT_H */
 