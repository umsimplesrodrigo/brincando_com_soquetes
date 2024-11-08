#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORTA 8080
#define TAMANHO_BUFFER 2048

DWORD WINAPI receber_mensagens(LPVOID arg) {
    SOCKET socket_cliente = *((SOCKET *)arg);
    char buffer[TAMANHO_BUFFER];
    int len;

    while ((len = recv(socket_cliente, buffer, TAMANHO_BUFFER, 0)) > 0) {
        buffer[len] = '\0';
        printf("Recebido: %s", buffer);
    }

    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET socket_cliente;
    struct sockaddr_in endereco_servidor;
    HANDLE handle_thread;
    char buffer[TAMANHO_BUFFER];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha no WSAStartup\n");
        exit(EXIT_FAILURE);
    }

    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_cliente == INVALID_SOCKET) {
        printf("Falha na criação do socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(PORTA);
    endereco_servidor.sin_addr.s_addr = inet_addr("192.168.0.103"); // Aqui eu usei o endereço IP local do meu computador para encontrar o servidor

    if (connect(socket_cliente, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor)) == SOCKET_ERROR) {
        printf("Falha na conexão\n");
        closesocket(socket_cliente);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Conectado ao servidor\n");

    handle_thread = CreateThread(NULL, 0, receber_mensagens, &socket_cliente, 0, NULL);
    if (handle_thread == NULL) {
        printf("Falha na criação da thread\n");
    } else {
        CloseHandle(handle_thread);
    }

    while (1) {
        fgets(buffer, TAMANHO_BUFFER, stdin);
        printf("Enviando: %s", buffer);
        send(socket_cliente, buffer, strlen(buffer), 0);
    }

    closesocket(socket_cliente);
    WSACleanup();
    return 0;
}
