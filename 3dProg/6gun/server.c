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
LONG res;
LPLONG lpres = &res;

int (*process_text)(char text[], char output[], int mod_count, int mod_max,
                    int buf_size);

void generate_filenames(CHAR name[], CHAR file1[], CHAR file2[]) {
    strcpy(file1, name);
    strcpy(file2, name);
    strcat(file1, ".txt");
    strcat(file2, ".dxd");
}

int SplitArguments(char *string, CHAR delim) {
    int i = 0;
    for (i = 0; i < lstrlen(string) - 1; i++) {
        if (string[i] == delim) {
            string[i] = '\0';
            break;
        }
    }
    return atoi(&string[i + 1]);
}

int ProcessResponse(char *resp) {

    HANDLE hIn, hOut, hLib;
    DWORD dIn, dOut;

    int rCount;
    char buf[BUF_SIZE], outbuf[BUF_SIZE];
    rCount = SplitArguments(resp, ' ');
    char inName[BUF_SIZE], outName[BUF_SIZE];
    generate_filenames(&resp[0], inName, outName);

    printf("<SERVER>: Filename is %s\n<SERVER>: Max changes is %d\n", inName,
           rCount);

    hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hIn == INVALID_HANDLE_VALUE) {
        printf("<SERVER>: Can't open input file: %lu\n", GetLastError());
        return -4;
    }

    hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        printf("<SERVER>: Can't open output file: %lu\n", GetLastError());
        return -5;
    }

    hLib = LoadLibrary("dlltest.dll");
    if (hLib == NULL) {
        printf("<SERVER>: Cannot load library");
        return -6;
    }
    process_text = (int (*)(char *, char *, int, int, int))GetProcAddress(
        hLib, "process_text");
    if (process_text == NULL) {
        printf("<SERVER>: process_text function not found");
        return -7;
    }

    int r = 0;
    while (ReadFile(hIn, buf, BUF_SIZE, &dIn, NULL) && dIn > 0) {
        r = (*process_text)(buf, outbuf, r, rCount, BUF_SIZE);
        WriteFile(hOut, outbuf, dIn - r, &dOut, NULL);
    }

    CloseHandle(hIn);
    CloseHandle(hOut);
    FreeLibrary(hLib);
    return r;
}

int main() {
    DWORD dReturnCode;
    char response[BUF_SIZE];

    hMutexSend = CreateMutex(NULL, FALSE, lpMutexSendName);
    hMutexRecv = CreateMutex(NULL, FALSE, lpMutexRecvName);

    if (hMutexSend == NULL || hMutexRecv == NULL) {
        printf("<SERVER>: Error <%lu> creating mutex\n", GetLastError());
        return 0;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        printf("<SERVER>: Seems that another server is already running\n");
        return 0;
    }

    hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, 100, lpFileShareName);

    if (hFileMapping == NULL) {
        printf("<SERVER>: Error <%lu> creating file mapping\n", GetLastError());
        return 0;
    }

    lpFileMap =
        MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

    if (lpFileMap == 0) {
        printf("<SERVER>: Error <%lu> mapping file\n", GetLastError());
    }

    printf("<SERVER>: Starting listening loop\n");

    while (1) {

        hMutexTermination = OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpMutexTermName);

        if (hMutexTermination == NULL) {
            continue;
        }

        printf("<SERVER>: Waiting for message\n");

        dReturnCode = WaitForSingleObject(hMutexTermination, 500);

        if (dReturnCode == WAIT_OBJECT_0 || dReturnCode == WAIT_FAILED) {
            break;
        }

        dReturnCode = WaitForSingleObject(hMutexRecv, INFINITE);
        dReturnCode = WaitForSingleObject(hMutexSend, INFINITE);

        if (!strcmp((LPSTR)lpFileMap, "")) {
            ReleaseMutex(hMutexSend);
            continue;
        } else if (!strcmp((LPSTR)lpFileMap, "exit")) {
            printf("<SERVER> Got termination signal\nExiting....\n");
            break;
        }

        printf("<SERVER>: Got command <<%s>>\n", (LPSTR)lpFileMap);

        int res = ProcessResponse(lpFileMap);
        if (res >= 0) {
            sprintf(response, "\n***<SERVER> Finnished with %d changes***\n",
                    res);
        } else {
          sprintf(response, "\n***<SERVER> Error processing file error %ld\n", GetLastError());
        }

        strcpy((char *)lpFileMap, response);

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
