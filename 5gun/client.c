#include <stdio.h>
#include <windows.h>

#define BUF_SIZE 512
#define MAX_CLIENTS 512

HANDLE MailSlot;

BOOL WriteSlot(LPSTR lpszMessage) {
    LPSTR Mail2 = "\\\\.\\mailslot\\$Channel2$";
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
        printf("WriteFile failed with %lu.\n", GetLastError());
        return FALSE;
    }

    printf("Slot written to successfully.\n");

    CloseHandle(hFile);
    return TRUE;
}

BOOL ReadSlot() {
    DWORD cbMessage, cMessage, cbRead;
    BOOL fResult;
    LPSTR lpszBuffer;

    cbMessage = cMessage = cbRead = 0;

    fResult = GetMailslotInfo(MailSlot,       // mailslot handle
                              (LPDWORD)NULL,  // no maximum message size
                              &cbMessage,     // size of next message
                              &cMessage,      // number of messages
                              (LPDWORD)NULL); // no read time-out

    if (!fResult) {
        printf("GetMailslotInfo failed with %lu.\n", GetLastError());
        return FALSE;
    }

    if (cbMessage == MAILSLOT_NO_MESSAGE) {
        printf("Waiting for a message...\n");
        return TRUE;
    }

    LPSTR response;
    while (cMessage != 0) {

        fResult = ReadFile(MailSlot, lpszBuffer, cbMessage, &cbRead, NULL);

        if (!fResult) {
            printf("ReadFile failed with %lu.\n", GetLastError());
            return FALSE;
        }

        // Display the message.

        printf("Contents of the mailslot: %s\n", lpszBuffer);

        printf("<SERVER>: \nResponse: \n");
        scanf("%s", response);
        if (!lstrcmp(response, "exit")) {
            return FALSE;
        }
        WriteSlot(response);

        fResult = GetMailslotInfo(MailSlot,       // mailslot handle
                                  (LPDWORD)NULL,  // no maximum message size
                                  &cbMessage,     // size of next message
                                  &cMessage,      // number of messages
                                  (LPDWORD)NULL); // no read time-out

        if (!fResult) {
            printf("GetMailslotInfo failed (%lu)\n", GetLastError());
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

    LPSTR Mail = "\\\\.\\mailslot\\$Channel1$";
    MakeSlot(Mail);
    if (MailSlot== INVALID_HANDLE_VALUE) {
        printf("<SERVER>: AAAAAAAAAAAAAAAAAAAAAAAAafailed with %lu\n", GetLastError());
        return FALSE;
    }
    while (TRUE) {
        if (!ReadSlot(MailSlot)) {
            break;
        };
        Sleep(3000);
    }
    return 0;
}
