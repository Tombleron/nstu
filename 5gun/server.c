#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define BUF_SIZE 512
#define MAX_CLIENTS 512

HANDLE MailSlot;
LPSTR Mail = "\\\\.\\mailslot\\$Channel2$";

int (*process_text)(char text[], char output[], int mod_count, int mod_max,
                    int buf_size);

void generate_filenames(CHAR name[], CHAR file1[], CHAR file2[]) {
    strcpy(file1, name);
    strcpy(file2, name);
    strcat(file1, ".txt");
    strcat(file2, ".dxd");
}

int SplitArguments(LPSTR string, CHAR delim) {
    int i = 0;
    for (i = 0; i < lstrlen(string) - 1; i++) {
        if (string[i] == delim) {
            string[i] = '\0';
            break;
        }
    }
    return i + 1;
}

int ProcessResponse(LPSTR resp) {

    HANDLE hIn, hOut, hLib;
    DWORD dIn, dOut;

    LPSTR buf, outbuf;

    LPSTR fArg = resp, sArg = resp + SplitArguments(resp, ' ');

    LPSTR inName, outName;
    generate_filenames(fArg, inName, outName);

    int rCount;
    rCount = atoi(sArg);

    printf("<SERVER>: Filename is %s\n<SERVER>: Max changes is %d", inName,
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

BOOL WriteSlot(LPSTR lpszMessage) {
    LPSTR Mail2 = "\\\\.\\mailslot\\$Channel1$";
    HANDLE hFile;

    hFile = CreateFile(Mail2, GENERIC_WRITE, FILE_SHARE_READ,
                       (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    BOOL fResult;
    DWORD cbWritten;

    fResult = WriteFile(hFile, lpszMessage,
                        (DWORD)(lstrlen(lpszMessage) + 1) * sizeof(TCHAR),
                        &cbWritten, (LPOVERLAPPED)NULL);

    if (!fResult) {
        printf("<SERVER>: WriteFile failed with %lu.\n", GetLastError());
        return FALSE;
    }

    printf("<SERVER>: Slot written to successfully.\n");

    CloseHandle(hFile);
    return TRUE;
}

BOOL ReadSlot() {
    DWORD cbMessage, cMessage, cbRead;
    BOOL fResult;
    LPTSTR lpszBuffer;

    cbMessage = cMessage = cbRead = 0;

    fResult = GetMailslotInfo(MailSlot,       // mailslot handle
                              (LPDWORD)NULL,  // no maximum message size
                              &cbMessage,     // size of next message
                              &cMessage,      // number of messages
                              (LPDWORD)NULL); // no read time-out

    if (!fResult) {
        printf("<SERVER>: GetMailslotInfo failed with %lu.\n", GetLastError());
        return FALSE;
    }

    if (cbMessage == MAILSLOT_NO_MESSAGE) {
        printf("<SERVER>: Waiting for a message...\n");
        return TRUE;
    }

    while (cMessage != 0) {

        ZeroMemory(lpszBuffer, BUF_SIZE);
        fResult = ReadFile(MailSlot, lpszBuffer, BUF_SIZE, &cbRead, NULL);

        if (!fResult) {
            printf("<SERVER>: ReadFile failed with %lu.\n", GetLastError());
            return FALSE;
        }

        // Display the message.
        printf("<SERVER>: Contents of the mailslot: %s\n", lpszBuffer);
        int r = ProcessResponse(lpszBuffer);
        printf("<SERVER>: Finished with %d changes\n", r);

        WriteSlot("exit");

        fResult = GetMailslotInfo(MailSlot,       // mailslot handle
                                  (LPDWORD)NULL,  // no maximum message size
                                  &cbMessage,     // size of next message
                                  &cMessage,      // number of messages
                                  (LPDWORD)NULL); // no read time-out

        if (!fResult) {
            printf("<SERVER>: GetMailslotInfo failed (%lu)\n", GetLastError());
            return FALSE;
        }
    }
    return TRUE;
}

BOOL MakeSlot(LPSTR lpszSlotName) {
    MailSlot = CreateMailslot(lpszSlotName, 0, MAILSLOT_WAIT_FOREVER,
                              (LPSECURITY_ATTRIBUTES)NULL);

    if (MailSlot == INVALID_HANDLE_VALUE) {
        printf("<SERVER>: CreateMailslot failed with %lu\n", GetLastError());
        return FALSE;
    } else {
        printf("<SERVER>: Mailslot created successfully.\n");
        return TRUE;
    }
}

int main() {

    MakeSlot(Mail);

    while (TRUE) {
        if (!ReadSlot(MailSlot)) {
            break;
        };
        Sleep(3000);
    }
    return 0;
}
