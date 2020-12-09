#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define BUF_SIZE 512
extern void SvcReportEvent(char*);

char DEFAULT_ADDR[15] = "127.0.0.1";
int DEFAULT_PORT = 27015;
int running = TRUE;

SOCKET serverSocket, clientSocket;
struct sockaddr_in localAddr;
struct sockaddr_in clientAddr;

typedef struct MyData {
    char num[15];
    SOCKET socket;
} MYDATA, *PMYDATA;

typedef enum MessageType {
    SERVER,
    SERVER_ERROR,
    THREAD,
    THREAD_ERROR,
} MessageType;

int (*process_text)(char text[], char output[], int mod_count, int mod_max,
                    int buf_size);

void DisplayMessage(MessageType type, char *message, char* num,
                    BOOL GetResponse) {
    char buf[BUF_SIZE];
    switch (type) {
    case SERVER:
        sprintf(buf, "<SERVER> %s", message);
        break;
    case SERVER_ERROR:
        sprintf(buf, "<SERVER> %s", message);
        break;
    case THREAD:
        sprintf(buf, "<THREAD #%s> %s", num, message);
        break;
    case THREAD_ERROR:
        sprintf(buf, "<THREAD №%s> %s", num, message);
        break;
    }
    SvcReportEvent(buf);
    if (GetResponse) {
        printf("%s", buf);
    }
}

void generate_filenames(char name[], char out[]) { strcat(out, name); }

int ProcessResponse(char *inName, int rCount, char* num, SOCKET funcSocket) {

    HANDLE hIn, hOut, hLib;
    DWORD dIn, dOut;

    char buf[BUF_SIZE], outbuf[BUF_SIZE];
    char outName[BUF_SIZE] = "out_";
    char message[BUF_SIZE];
    generate_filenames(inName, outName);

    hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hIn == INVALID_HANDLE_VALUE) {
        sprintf(message, "Can't open input file\n");
        DisplayMessage(THREAD_ERROR, message, num, FALSE);
        send(funcSocket, message, sizeof(message), 0);
        closesocket(funcSocket);
        return 0;
    }

    hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        sprintf(message, "Can't open output file\n");
        DisplayMessage(THREAD_ERROR, message, num, FALSE);
        send(funcSocket, message, sizeof(message), 0);
        closesocket(funcSocket);
        return 0;
    }

    DisplayMessage(THREAD, "Trying to load library\n", num, FALSE);
    hLib = LoadLibrary("dlltest.dll");
    if (hLib == NULL) {
        sprintf(message, "Unable to load library\n");
        DisplayMessage(THREAD_ERROR, message, num, FALSE);
        send(funcSocket, message, sizeof(message), 0);
        closesocket(funcSocket);
        return 0;
    }

    process_text = (int (*)(char *, char *, int, int, int))GetProcAddress(
        hLib, "process_text");
    if (process_text == NULL) {
        sprintf(message, "process_text function not found\n");
        DisplayMessage(THREAD_ERROR, message, num, FALSE);
        send(funcSocket, message, sizeof(message), 0);
        closesocket(funcSocket);
        return 0;
    }

    DisplayMessage(THREAD, "Processing text\n", num, FALSE);
    int r = 0;
    while (ReadFile(hIn, buf, BUF_SIZE, &dIn, NULL) && dIn > 0) {
        r = (*process_text)(buf, outbuf, r, rCount, BUF_SIZE);
        WriteFile(hOut, outbuf, dIn - r, &dOut, NULL);
    }
    sprintf(outbuf, "Finished with %d changes\n", r);
    DisplayMessage(THREAD, outbuf, num, FALSE);

    CloseHandle(hIn);
    CloseHandle(hOut);
    FreeLibrary(hLib);
    return r;
}

DWORD WINAPI clientProcessing(LPVOID data) {

    PMYDATA myData = (PMYDATA)data;
    SOCKET funcSocket = myData->socket;
    char num[15];
    strcpy(num, myData->num);
    char buff[BUF_SIZE];
    char str1[] = "Enter the file name: ";
    char str2[] = "Enter the maximum of changes: ";
    int cbWritten, changes;
    char message[BUF_SIZE];

    char fileName[BUF_SIZE];
    int maxChanges = 0;

    ZeroMemory(message, BUF_SIZE);
    while (1) {
        ZeroMemory(buff, BUF_SIZE);
        strcat(message, str1);
        send(funcSocket, message, sizeof(message), 0);
        ZeroMemory(message, BUF_SIZE);
        cbWritten = recv(funcSocket, buff, sizeof(buff), 0);
        if (cbWritten == SOCKET_ERROR) // принятие сообщения от клиента
        {
            sprintf(message,
                    "Error while receiving data from client [ERROR #%ld].\n",
                    GetLastError());
            DisplayMessage(THREAD_ERROR, message, num, FALSE);
            send(funcSocket, message, sizeof(message), 0);
            closesocket(funcSocket);
            ExitThread(-3);
        }
        if (!strcmp(buff, "exit"))
            break;
        strcpy(fileName, buff);

        ZeroMemory(buff, BUF_SIZE);
        send(funcSocket, str2, sizeof(str2), 0); // отправляем клиенту сообщение
        cbWritten = recv(funcSocket, buff, sizeof(buff), 0);
        if (cbWritten == SOCKET_ERROR) // принятие сообщения от клиента
        {
            sprintf(message,
                    "Error while receiving data from clinet [ERROR #%ld].\n",
                    GetLastError());
            DisplayMessage(THREAD_ERROR, message, num, FALSE);
            send(funcSocket, message, sizeof(message), 0);
            closesocket(funcSocket);
            ExitThread(-4);
        }
        if (!strcmp(buff, "exit"))
            break;
        maxChanges = atoi(buff);

        changes = ProcessResponse(fileName, maxChanges, num, funcSocket);

        sprintf(message, "File \"%s\" processed with total changes of %d ",
                fileName, changes);
        DisplayMessage(THREAD, message, num, FALSE);
    }

    closesocket(funcSocket);
    return 0;
}

int ServiceStart(int argc, char**argv) {

    if (argc == 3) {
      DEFAULT_PORT = atoi(argv[2]);
      strcpy(DEFAULT_ADDR, argv[1]);
    } else if (argc != 1) {
      DisplayMessage(SERVER_ERROR, "Usage ./tcp_server.exe [address] [port]", 0, FALSE);
      return -1;
    }

    char buff[1024];
    char message[BUF_SIZE];
    DisplayMessage(SERVER, "Starting server\n", 0,  FALSE);

    if (WSAStartup(0x0202, (WSADATA *)&buff[0])) {
        sprintf(message, "Error with WSAStartup [ERROR #%d].\n",
               WSAGetLastError());
        DisplayMessage(SERVER_ERROR, message, 0, FALSE);
        return -1;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        sprintf(message, "Error while creating a socket [ERROR #%d].\n",
               WSAGetLastError());
        DisplayMessage(SERVER_ERROR, message, 0, FALSE);
        WSACleanup();
        return -2;
    }

    localAddr.sin_family = AF_INET; 
    localAddr.sin_port = htons(DEFAULT_PORT);
    localAddr.sin_addr.s_addr =
        inet_addr(DEFAULT_ADDR);

    if (bind(serverSocket, (struct sockaddr *)&localAddr, sizeof(localAddr))) {
        sprintf(message, "Error while binding the socket and address "
               "[ERROR #%d].\n",
               WSAGetLastError());
        DisplayMessage(SERVER_ERROR, message, 0, FALSE);
        closesocket(serverSocket);
        WSACleanup();
        return -3;
    }

    if (listen(serverSocket, 0x100)) {
        sprintf(message, "Error with listening to the socket [ERROR #%d]\n",
               WSAGetLastError());
        DisplayMessage(SERVER_ERROR, message, 0, FALSE);
        closesocket(serverSocket);
        WSACleanup();
        return -4;
    }
}

int Server() {
    int clientAddrSize = sizeof(clientAddr);
    char message[BUF_SIZE];
    while ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                                 &clientAddrSize))) {
        if (running == FALSE) {
            break;
        }
        HOSTENT *hst;
        hst = gethostbyaddr((char *)&clientAddr.sin_addr.s_addr, 4, AF_INET);
        sprintf(message, "New connection established: \"%s\".\n",
               (hst) ? hst->h_name : "");
        DWORD thID;
        MYDATA data;
        strcpy(data.num, (hst) ? hst->h_name : "");
        data.socket = clientSocket;

        CreateThread(NULL, 0, clientProcessing, &data, 0, &thID);
    }
    return 0;
}

void ServiceStop() {
    running = FALSE;
    SvcReportEvent("Service stopped\n");
}
