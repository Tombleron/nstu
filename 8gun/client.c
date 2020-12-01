#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define BUF_SIZE 512

int DEFAULT_PORT = 27015;
char DEFAULT_ADDR[15] = "127.0.0.1";

typedef enum MessageType {
    CLIENT,
    CLIENT_ERROR,
} MessageType;

void DisplayMessage(MessageType type, char *message, BOOL GetResponse) {
    switch (type) {
    case CLIENT:
        printf("\033[1;38m");
        printf("<CLIENT> %s", message);
        printf("\033[0m");
        break;
    case CLIENT_ERROR:
        printf("\033[1;31m");
        printf("<CLIENT> %s", message);
        printf("\033[0m");
        break;
    }
    if (GetResponse) {
        scanf("%s", message);
        printf("%s", message);
    }
}

int main(int argc, char *argv[]) {

        printf("<SERVER>");
    if (argc == 3) {
      DEFAULT_PORT = atoi(argv[2]);
      strcpy(DEFAULT_ADDR, argv[1]);
    } else if (argc != 1) {
      DisplayMessage(CLIENT_ERROR, "Usage ./tcp_server.exe [address] [port]", FALSE);
      return -1;
    }
    int cbWritten;
    char buff[1024], message[BUF_SIZE];

    if (WSAStartup(0x0202, (WSADATA *)&buff[0])) {
        sprintf(message, "Error with WSAStartup [ERROR #%d].\n",
               WSAGetLastError());
      DisplayMessage(CLIENT_ERROR, message, FALSE);
        return -1;
    }

    SOCKET mySocket;
    if ((mySocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        sprintf(message, "Error while creating a socket [ERROR #%d].\n",
               WSAGetLastError());
      DisplayMessage(CLIENT_ERROR, message, FALSE);
        WSACleanup();
        return -2;
    }

    struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(DEFAULT_PORT);

    if (inet_addr(DEFAULT_ADDR) != INADDR_NONE)
        destAddr.sin_addr.s_addr = inet_addr(DEFAULT_ADDR);
    else {
        sprintf(message, "[CLIENT] Can't read network address.\n");
      DisplayMessage(CLIENT_ERROR, message, FALSE);
        return -3;
    }

    if (connect(mySocket, (struct sockaddr *)&destAddr, sizeof(destAddr))) {
        sprintf("Can't connect to the server \"%s:%d\" [ERROR #%d].\n",
               DEFAULT_ADDR, DEFAULT_PORT, WSAGetLastError());
      DisplayMessage(CLIENT_ERROR, message, FALSE);
        return -4;
    }

    sprintf(message, "Connected to the server \"%s:%d\".\n", DEFAULT_ADDR,
           DEFAULT_PORT);
    DisplayMessage(CLIENT, message, FALSE);

    while (1) {
        cbWritten = recv(mySocket, buff, sizeof(buff), 0);
        if (cbWritten == SOCKET_ERROR) {
            sprintf(message, "Can't receive data from server [ERROR #%d].\n",
                   WSAGetLastError());
      DisplayMessage(CLIENT_ERROR, message, FALSE);
            return -1;
        }

        printf("\033[1;38m");
        printf("<SERVER> %s", buff);
        printf("\033[0m");
        printf("\033[1;32m");
        printf("[USER] ");
        printf("\033[0m");
        ZeroMemory(buff, 1024);
        scanf("%s", buff);

        send(mySocket, buff, strlen(buff), 0);
        if (!strcmp(buff, "exit"))
            break;
    }
    closesocket(mySocket);
    WSACleanup();
    return 0;
}
