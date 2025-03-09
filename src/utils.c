#include "utils.h"

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

bool isValidPort(char *port)
{
    int size = strlen(port);
    for (int i = 0; i < size; i++)
        if (port[i] < '0' || port[i] > '9')
            return false;

    int portNumber = atoi(port);
    return portNumber > 0 && portNumber <= 65535;
}

void show_help() {
    printf("Comandos Disponíveis:\n");
    printf("  join/j <net>                 - Entrar na rede solicitando a lista de nós ao servidor de nós.\n");
    printf("  direct join <IP> <Porta> ou dj <IP> <Porta> - Entrar na rede conectando diretamente a um nó específico.\n");
    printf("  create <nome> ou c <nome>  - Criar um objeto com o nome especificado.\n");
    printf("  delete <nome> ou dl <nome> - Deletar o objeto com o nome especificado.\n");
    printf("  retrieve <nome> ou r <nome> - Recuperar o objeto com o nome especificado.\n");
    printf("  show names ou sn            - Mostrar os nomes de todos os objetos armazenados no nó.\n");
    printf("  show interest table ou si   - Mostrar todas as entradas da tabela de interesses pendentes.\n");
    printf("  show topology ou st          - Mostrar a topologia da rede.\n");
    printf("  leave ou l                   - Sair da rede.\n");
    printf("  exit ou x                    - Fechar a aplicação.\n");
    printf("  help ou h                    - Exibir esta mensagem de ajuda.\n");
}

