#include <stdio.h>
#include <string.h>
#include <windows.h>

#define BUF_SIZE 256

HINSTANCE hLib;

HANDLE hMutexSend, hMutexRecv, hMutexTermination;

CHAR lpMutexSendName[] = "$MyMutexSendName$",
     lpMutexRecvName[] = "$MyMutexRecvName$",
     lpMutexTermName[] = "$MyMutexTermName$",
     lpFileShareName[] = "$MyFileShareName$";

HANDLE hFileMapping;

LPVOID lpFileMap;

int (*process_text)(char text[], char output[], int mod_count, int mod_max,
                    int buf_size);

int main() {
    DWORD dReturnCode;
    char response[BUF_SIZE], number[BUF_SIZE];

    hMutexSend = OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpMutexSendName);
    hMutexRecv = OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpMutexRecvName);

    if (hMutexSend == NULL || hMutexRecv == NULL) {
        printf("<CLIENT>: Error <%lu> openning mutex\n", GetLastError());
        return 0;
    }

    hMutexTermination = CreateMutex(0, TRUE, lpMutexTermName);

    if (hMutexTermination == NULL) {
        printf("<CLIENT>: Error <%lu> creating termination mutex\n",
               GetLastError());
        return 0;
    }

    hFileMapping =
        OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, lpFileShareName);

    if (hFileMapping == NULL) {
        printf("<CLIENT>: Error <%lu> openning file mapping\n", GetLastError());
        return 0;
    }

    lpFileMap =
        MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

    if (lpFileMap == 0) {
        printf("<CLIENT>: Error <%lu> mapping file\n", GetLastError());
        return 0;
    }

    while (1) {

        dReturnCode = WaitForSingleObject(hMutexSend, INFINITE);

        printf("<CLIENT>: Enter name of file: ");
        scanf("%s", response);

        if (!lstrcmp(response, "exit")) {
            strcpy((char *)lpFileMap, "exit");
            break;
        }

        printf("<CLIENT>: Enter max counts: ");
        scanf("%s", number);

        lstrcat(response, " ");
        lstrcat(response, number);

        strcpy((char *)lpFileMap, response);

        ReleaseMutex(hMutexSend);
        ReleaseMutex(hMutexRecv);
        dReturnCode = WaitForSingleObject(hMutexRecv, INFINITE);
        dReturnCode = WaitForSingleObject(hMutexSend, INFINITE);

        if (!strcmp((LPSTR)lpFileMap, "")) {
            ReleaseMutex(hMutexSend);
            continue;
        }

        dReturnCode = WaitForSingleObject(hMutexRecv, INFINITE);

        if (dReturnCode == WAIT_OBJECT_0) {
            printf("<CLIENT>: Server's response is %s", (char *)lpFileMap);
        } else {
            printf("<CLIENT>: Error retieving response\n");
            break;
        }
        ReleaseMutex(hMutexSend);
        ReleaseMutex(hMutexRecv);
    }

    CloseHandle(hMutexSend);
    CloseHandle(hMutexRecv);
    CloseHandle(hMutexTermination);

    UnmapViewOfFile(lpFileMap);

    CloseHandle(hFileMapping);

    return 0;
}
