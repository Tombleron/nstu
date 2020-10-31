#include <stdio.h>
#include <windows.h>

int main() {
    BOOL fReturnCode;

    DWORD cbMessages;
    DWORD cbMsgNumber;

    HANDLE hMailslot1, hMailslot2;

    LPSTR lpszReadMailslotName = "\\\\.\\mailslot\\$Channel1$";
    LPSTR lpszWriteMailslotName = "\\\\*\\mailslot\\$Channel2$";

    char szBuf[512];
    DWORD cbRead;
    DWORD cbWritten;
    DWORD total = 0;

    char message[80] = {0};

    FILE *hdl;

    hMailslot1 =
        CreateMailslot(lpszReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);

    if (hMailslot1 == INVALID_HANDLE_VALUE) {
        fprintf(stdout, "CreateMailslot: Error %ld\n", GetLastError());
        return 0;
    }

    fprintf(stdout, "Mailslot created\n");
    while (1) {
        fReturnCode =
            GetMailslotInfo(hMailslot1, NULL, &cbMessages, &cbMsgNumber, NULL);
        if (!fReturnCode) {
            fprintf(stdout, "GetMailslotInfo: Error %ld\n", GetLastError());
            break;
        }

        if (cbMsgNumber != 0) {
            if (ReadFile(hMailslot1, szBuf, 512, &cbRead, NULL)) {
                printf("Received: <%s>\n", szBuf);
                if (!strcmp(szBuf, "exit"))
                    break;
                else {
                    hMailslot2 = CreateFile(lpszWriteMailslotName,
                                            GENERIC_WRITE, FILE_SHARE_READ,
                                            NULL, OPEN_EXISTING, 0, NULL);
                    if (hMailslot2 == INVALID_HANDLE_VALUE) {
                        fprintf(stdout, "CreateFile for send: Error %ld\n",
                                GetLastError());
                        _getch();
                        break;
                    }
                    if (hdl = fopen(szBuf, "rt")) {
                        while (!feof(hdl)) {
                            if ((char)fgetc(hdl) == 0x20)
                                total++;
                        }
                        sprintf(message, "(Server): file:%s, spaces = %d\n",
                                szBuf, total);
                        WriteFile(GetStdHandle(STD_ERROR_HANDLE), message,
                                  strlen(message), &cbWritten, NULL);
                        sprintf(message, "%d", total);
                        WriteFile(hMailslot2, message, strlen(message) + 1,
                                  &cbWritten, NULL);
                        printf("Bytes sent %d\n", cbWritten);
                        fclose(hdl);
                    } else {
                        sprintf(message, "(Server)Can't open %s!", szBuf);
                        WriteFile(GetStdHandle(STD_ERROR_HANDLE), message,
                                  strlen(message) + 1, &cbWritten, NULL);
                        printf("\n");
                        WriteFile(hMailslot2, message, strlen(message) + 1,
                                  &cbWritten, NULL);
                        printf("Bytes sent %d\n", cbWritten);
                    }
                }
            } else {
                fprintf(stdout, "ReadFile: Error %ld\n", GetLastError());
                getch();
                break;
            }
        }
        Sleep(500);
    }
    CloseHandle(hMailslot1);
    CloseHandle(hMailslot2);
    return 0;
}
