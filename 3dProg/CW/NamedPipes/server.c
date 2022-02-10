#include <stdio.h>
#include <string.h>
#include <windows.h>

#define BUFSIZE 512

DWORD WINAPI Process(LPVOID);
VOID GetAnswerToRequest(char *, char *, LPDWORD);
int (*process_text)(char text[], char output[], int mod_count, char ch);
void generate_filenames(char name[], char out[]);
void ProcessResponse(char *inName, char ch, char *message);
extern int SvcReportEvent(const char *text);

BOOL running;
BOOL fConnected = FALSE;
HANDLE hPipe = INVALID_HANDLE_VALUE;
char *lpszPipename = "\\\\.\\pipe\\mynamedpipe";

int ServiceStart() { running = TRUE; }

int Server() {

  char message[BUFSIZE];

    while (running) {
          sprintf(message,"\nPipe Server awaiting client connection "
               "on %s\n",
               lpszPipename);
        SvcReportEvent(message);
        hPipe = CreateNamedPipe(lpszPipename,               // pipe name
                                PIPE_ACCESS_DUPLEX,         // read/write access
                                PIPE_TYPE_MESSAGE |         // message type pipe
                                    PIPE_READMODE_MESSAGE | // message-read mode
                                    PIPE_WAIT,              // blocking mode
                                PIPE_UNLIMITED_INSTANCES,   // max. instances
                                BUFSIZE, // output buffer size
                                BUFSIZE, // input buffer size
                                0,       // client time-out
                                NULL);   // default security attribute

        if (hPipe == INVALID_HANDLE_VALUE) {
              sprintf(message, "CreateNamedPipe failed, GLE=%ld.\n", GetLastError());
            SvcReportEvent(message);
            return -1;
        }

        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

        fConnected = ConnectNamedPipe(hPipe, NULL)
                         ? TRUE
                         : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected) {
            SvcReportEvent("Client connected, processing...\n");
        } else
            // The client could not connect, so close the pipe.
            CloseHandle(hPipe);

        Process(hPipe);
    }

    return 0;
}

void ServiceStop() {
    running = FALSE;
    CreateFile(lpszPipename,  // pipe name
                           GENERIC_READ | // read and write access
                               GENERIC_WRITE,
                           0,             // no sharing
                           NULL,          // default security attributes
                           OPEN_EXISTING, // opens existing pipe
                           0,             // default attributes
                           NULL);         // no template file
    CloseHandle(hPipe);
}

DWORD WINAPI Process(HANDLE hPipe) {
    char message[BUFSIZE];
    char pchRequest[BUFSIZE];
    char pchReply[BUFSIZE];

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;

    // Print verbose messages. In production code, this should be for debugging
    // only.
    SvcReportEvent("Receiving and processing messages.\n");

    // Loop until done reading
    while (1) {
        // Read client requests from the pipe. This simplistic code only allows
        // messages up to BUFSIZE characters in length.
        fSuccess = ReadFile(hPipe,                   // handle to pipe
                            pchRequest,              // buffer to receive data
                            BUFSIZE, // size of buffer
                            &cbBytesRead,            // number of bytes read
                            NULL);                   // not overlapped I/O

        if (!fSuccess || cbBytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                SvcReportEvent("Client disconnected.\n");
            } else {
                sprintf(message, "ReadFile failed, GLE=%ld.\n", GetLastError());
                SvcReportEvent(message);
            }
            break;
        }

        // Process the incoming message.
        GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);

        // Write the reply to the pipe.
        fSuccess = WriteFile(hPipe,        // handle to pipe
                             pchReply,     // buffer to write from
                             cbReplyBytes, // number of bytes to write
                             &cbWritten,   // number of bytes written
                             NULL);        // not overlapped I/O

        if (!fSuccess || cbReplyBytes != cbWritten) {
            
              sprintf(message, "WriteFile failed, GLE=%ld.\n", GetLastError());
            SvcReportEvent(message);
            break;
        }
    }

    // Flush the pipe to allow the client to read the pipe's contents
    // before disconnecting. Then disconnect the pipe, and close the
    // handle to this pipe instance.

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    SvcReportEvent("Exiting...\n");
    return 1;
}

VOID GetAnswerToRequest(char *pchRequest, char *pchReply, LPDWORD pchBytes) {
  char message[BUFSIZE];
    sprintf(message, "Client Request String:\"%s\"\n", pchRequest);
    SvcReportEvent(message);
    int len = strlen(pchRequest);
    pchRequest[len - 2] = '\0';
    ProcessResponse(pchRequest, pchRequest[len - 1], pchReply);
    SvcReportEvent(pchReply);
    *pchBytes = strlen(pchReply) + 1;
}

void generate_filenames(char name[], char out[]) {
    strcpy(out, name);
    strcat(out, "_out");
}

void ProcessResponse(char *inName, char ch, char *message) {

    HANDLE hIn, hOut, hLib;
    DWORD dIn, dOut;

    char buf[BUFSIZE], outbuf[BUFSIZE];
    char outName[BUFSIZE];
    generate_filenames(inName, outName);

    hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hIn == INVALID_HANDLE_VALUE) {
        sprintf(message, "Can't open input file");
        return;
    }

    hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        sprintf(message, "Can't open output file");
        return;
    }

    SvcReportEvent("Trying to load library\n");
    hLib = LoadLibrary("dlltest.dll");
    if (hLib == NULL) {
        sprintf(message, "Unable to load library");
        return;
    }

    process_text = (int (*)(char *, char *, int, char))GetProcAddress(
        hLib, "process_text");
    if (process_text == NULL) {
        sprintf(message, "process_text function not found");
        return;
    }

    SvcReportEvent("Processing text\n");
    int r = 0;
    while (ReadFile(hIn, buf, BUFSIZE, &dIn, NULL) && dIn > 0) {
        r = (*process_text)(buf, outbuf, r, ch);
        WriteFile(hOut, outbuf, dIn - r, &dOut, NULL);
    }
    sprintf(message, "Finished with %d changes", r);

    CloseHandle(hIn);
    CloseHandle(hOut);
    FreeLibrary(hLib);
}
