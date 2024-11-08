#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORTA 8080
#define TAMANHO_BUFFER 2048

SOCKET sockets_clientes[100];
int num_clientes = 0;

void enviar_mensagem_para_todos_os_clientes(char *mensagem, SOCKET socket_remetente) {
    for (int i = 0; i < num_clientes; ++i) {
        if (sockets_clientes[i] != INVALID_SOCKET && sockets_clientes[i] != socket_remetente) {
            if (send(sockets_clientes[i], mensagem, strlen(mensagem), 0) < 0) {
                perror("Erro ao enviar mensagem para o cliente");
            }
        }
    }
}

DWORD WINAPI lidar_com_cliente(LPVOID arg) {
    SOCKET socket_cliente = *((SOCKET *)arg);
    char buffer[TAMANHO_BUFFER];
    int len;

    while ((len = recv(socket_cliente, buffer, TAMANHO_BUFFER, 0)) > 0) {
        buffer[len] = '\0';
        printf("Recebido do cliente %d: %s\n", socket_cliente, buffer);
        enviar_mensagem_para_todos_os_clientes(buffer, socket_cliente);
    }

    if (len == 0) {
        printf("Cliente %d desconectado\n", socket_cliente);
    } else {
        printf("recv() falhou com erro: %d\n", WSAGetLastError());
    }

    closesocket(socket_cliente);

    for (int i = 0; i < num_clientes; ++i) {
        if (sockets_clientes[i] == socket_cliente) {
            sockets_clientes[i] = INVALID_SOCKET;
            break;
        }
    }

    free(arg);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET socket_servidor, novo_socket;
    struct sockaddr_in endereco_servidor, endereco_cliente;
    int tamanho_endereco = sizeof(struct sockaddr_in);
    HANDLE handle_thread;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha no WSAStartup\n");
        exit(EXIT_FAILURE);
    }
    
    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_servidor == INVALID_SOCKET) {
        printf("Falha na criação do socket\n");
        exit(EXIT_FAILURE);
    }

    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_addr.s_addr = INADDR_ANY;
    endereco_servidor.sin_port = htons(PORTA);

    if (bind(socket_servidor, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor)) == SOCKET_ERROR) {
        printf("Falha no bind\n");
        closesocket(socket_servidor);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(socket_servidor, SOMAXCONN) == SOCKET_ERROR) {
        printf("Falha no listen\n");
        closesocket(socket_servidor);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("O servidor está ouvindo na porta %d\n", PORTA);

    while (1) {
        novo_socket = accept(socket_servidor, (struct sockaddr *)&endereco_cliente, &tamanho_endereco);
        if (novo_socket == INVALID_SOCKET) {
            printf("Falha no accept\n");
            continue;
        }
        printf("Aceita uma nova conexão\n");

        sockets_clientes[num_clientes++] = novo_socket;
        handle_thread = CreateThread(NULL, 0, lidar_com_cliente, &sockets_clientes[num_clientes - 1], 0, NULL);
        if (handle_thread == NULL) {
            printf("Falha na criação da thread\n");
            closesocket(novo_socket);
        } else {
            CloseHandle(handle_thread);
        }
    }

    closesocket(socket_servidor);
    WSACleanup();
    return 0;
}
