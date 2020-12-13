#include <stdio.h>
#include <string.h>
#include <windows.h>

#define BUFSIZE 512

int main() {
    HANDLE hPipe;
    char lpvMessage[BUFSIZE];
    char chBuf[BUFSIZE];
    BOOL fSuccess = FALSE;
    DWORD cbRead, cbToWrite, cbWritten, dwMode;
    char *lpszPipename = "\\\\.\\pipe\\mynamedpipe";

    // Try to open a named pipe; wait for it, if necessary.
    while (1) {
        hPipe = CreateFile(lpszPipename,  // pipe name
                           GENERIC_READ | // read and write access
                               GENERIC_WRITE,
                           0,             // no sharing
                           NULL,          // default security attributes
                           OPEN_EXISTING, // opens existing pipe
                           0,             // default attributes
                           NULL);         // no template file

        // Break if the pipe handle is valid.
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY) {
            printf("Could not open pipe. GLE=%ld\n", GetLastError());
            return -1;
        }

        // All pipe instances are busy, so wait for 20 seconds.
        if (!WaitNamedPipe(lpszPipename, 20000)) {
            printf("Could not open pipe: 20 second wait timed out.");
            return -1;
        }
    }

    printf("Connected to the pipe\n");

    // The pipe connected; change to message-read mode.

    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(hPipe,   // pipe handle
                                       &dwMode, // new pipe mode
                                       NULL,    // don't set maximum bytes
                                       NULL);   // don't set maximum time
    if (!fSuccess) {
        printf("SetNamedPipeHandleState failed. GLE=%ld\n", GetLastError());
        return -1;
    }

    char c[BUFSIZE];
    char filename[BUFSIZE];
    // Send a message to the pipe server.
    while (TRUE) {
        printf("Input filename: ");
        scanf("%s", filename);
        if (!strcmp(filename, "exit")) {
            break;
        }
        printf("Input char: ");
        scanf(" %c", c);
        sprintf(lpvMessage, "%s %c", filename, c[0]);
        printf("Sending %d byte message: \"%s\"\n", strlen(lpvMessage) + 1,
               lpvMessage);

        fSuccess = WriteFile(hPipe,      // pipe handle
                             lpvMessage, // message
                             cbToWrite,  // message length
                             &cbWritten, // bytes written
                             NULL);      // not overlapped

        if (!fSuccess) {
            printf("WriteFile to pipe failed. GLE=%ld\n", GetLastError());
            return -1;
        }

        printf("\nMessage sent to server, receiving reply as follows:\n");

        do {
            // Read from the pipe.
            fSuccess = ReadFile(hPipe,   // pipe handle
                                chBuf,   // buffer to receive reply
                                BUFSIZE, // size of buffer
                                &cbRead, // number of bytes read
                                NULL);   // not overlapped

            if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
                break;

            printf("\"%s\"\n", chBuf);
        } while (!fSuccess); // repeat loop if ERROR_MORE_DATA

        if (!fSuccess) {
            printf("ReadFile from pipe failed. GLE=%ld\n", GetLastError());
            return -1;
        }

        printf("\n<End of message>\n");
    }
    CloseHandle(hPipe);
    return 0;
}
