#include <stdio.h>
#include <string.h>
#include <windows.h>

#define MAX_THREADS 8
#define BUF_SIZE 256
#define LIB_NAME "dlltest.dll"

typedef struct MyData {
    int num;
    char file[80];
    int changes;
} MYDATA, *PMYDATA;

typedef enum MessageType {
    SERVER,
    SERVER_ERROR,
    THREAD,
    THREAD_ERROR,
} MessageType;

int (*process_text)(char text[], char output[], int mod_count, int mod_max,
                    int buf_size);
int ProcessResponse(char *inName, int rCount, int num);
void generate_filenames(char name[], char out[]);
void DisplayMessage(MessageType type, char *message, int num, BOOL GetResponse);
DWORD WINAPI MyThreadFunction(LPVOID lpParam);

HANDLE hMutex;

int main(int argc, char *argv[]) {
    MYDATA pDataArray[MAX_THREADS];
    DWORD dwThreadIdArray[MAX_THREADS];
    HANDLE hThreadArray[MAX_THREADS];
    char message[BUF_SIZE] =
        "Not enough arguments. There must be at least 3 of them. Usage: "
        "./thread.exe [changes] [filenames]";

    if (argc < 3) {
        DisplayMessage(SERVER_ERROR, message, 0, FALSE);
        exit(-1);
    }

    if (argc > MAX_THREADS) {
        DisplayMessage(
            SERVER_ERROR,
            "Too many arguments, the program will process only few\n", 0,
            FALSE);
        argc = MAX_THREADS + 2;
    }

    DisplayMessage(SERVER, "Creating mutex\n", 0, FALSE);
    hMutex = CreateMutex(NULL, FALSE, NULL);
    DisplayMessage(SERVER, "Capturing the mutex\n", 0, FALSE);
    WaitForSingleObject(hMutex, INFINITE);

    for (int i = 2; i < argc; i++) {

        pDataArray[i - 2].num = i - 1;
        strcpy(pDataArray[i - 2].file, argv[i]);
        pDataArray[i - 2].file[strlen(argv[i])] = '\0';
        pDataArray[i - 2].changes = atoi(argv[1]);

        // Create the thread to begin execution on its own.
        sprintf(message, "Starting thread #%d\n", i - 1);
        DisplayMessage(SERVER, message, 0, FALSE);

        hThreadArray[i] =
            CreateThread(NULL,                 // default security attributes
                         0,                    // use default stack size
                         MyThreadFunction,     // thread function name
                         &pDataArray[i - 2],   // argument to thread function
                         0,                    // use default creation flags
                         &dwThreadIdArray[i]); // returns the thread identifier

        // Check the return value for success.
        // If CreateThread fails, terminate execution.
        // This will automatically clean up threads and memory.

        if (hThreadArray[i] == NULL) {
            DisplayMessage(SERVER_ERROR, "Error creating thread\n", 0, FALSE);
            ExitProcess(1);
        }
        sprintf(message, "Thread #%d successfully started\n", i - 1);
        DisplayMessage(SERVER, message, 0, FALSE);
    } // End of main thread creation loop.

    DisplayMessage(SERVER, "Releasing mutex\n", 0, FALSE);
    ReleaseMutex(hMutex);

    // Wait until all threads have terminated.
    DisplayMessage(SERVER, "Waiting for threads to finish\n", 0, FALSE);
    for (int i = 0; i < argc - 1; i++) {
        WaitForSingleObject(hThreadArray[i], INFINITE);
    }
    // Sleeping beacause output of thread sometimes can't get in time
    // Sleep(30000);

    // Close all thread handles.
    CloseHandle(hMutex);

    for (int i = 0; i < MAX_THREADS; i++) {
        CloseHandle(hThreadArray[i]);
    }

    return 0;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {

    PMYDATA pDataArray;
    char message[BUF_SIZE];

    // Cast the parameter to the correct data type.
    // The pointer is known to be valid because
    // it was checked for NULL before the thread was created.
    pDataArray = (PMYDATA)lpParam;

    DisplayMessage(THREAD, "Waiting for the mutex\n", pDataArray->num, FALSE);
    WaitForSingleObject(hMutex, INFINITE);
    DisplayMessage(THREAD, "Captured the mutex\n", pDataArray->num, FALSE);

    // Print the parameter values.
    sprintf(message, "File: %s, Changes: %d\n", pDataArray->file,
            pDataArray->changes);
    DisplayMessage(THREAD, message, pDataArray->num, FALSE);

    pDataArray->changes = ProcessResponse(pDataArray->file, pDataArray->changes, pDataArray->num);

    DisplayMessage(THREAD, "Releasing the mutex\n", pDataArray->num, FALSE);
    ReleaseMutex(hMutex);

    return 0;
}

void generate_filenames(char name[], char out[]) { strcat(out, name); }

int ProcessResponse(char *inName, int rCount, int num) {

    HANDLE hIn, hOut, hLib;
    DWORD dIn, dOut;

    char buf[BUF_SIZE], outbuf[BUF_SIZE];
    char outName[BUF_SIZE] = "out_";
    generate_filenames(inName, outName);

    hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hIn == INVALID_HANDLE_VALUE) {
        DisplayMessage(THREAD_ERROR, "Can't open input file\n", num, FALSE);
        return 0;
    }

    hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        DisplayMessage(THREAD_ERROR, "Can't open output file\n", num, FALSE);
        ReleaseMutex(hMutex);
        return 0;
    }

    DisplayMessage(THREAD, "Trying to load library\n", num, FALSE);
    hLib = LoadLibrary("dlltest.dll");
    if (hLib == NULL) {
        DisplayMessage(THREAD_ERROR, "Unable to load library\n", num, FALSE);
        return 0;
    }

    process_text = (int (*)(char *, char *, int, int, int))GetProcAddress(
        hLib, "process_text");
    if (process_text == NULL) {
        DisplayMessage(THREAD_ERROR, "process_text function not found\n", num,
                       FALSE);
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

void DisplayMessage(MessageType type, char *message, int num,
                    BOOL GetResponse) {
    switch (type) {
    case SERVER:
        printf("\033[1;38m");
        printf("<SERVER> %s", message);
        printf("\033[0m");
        break;
    case SERVER_ERROR:
        printf("\033[1;31m");
        printf("<SERVER> %s", message);
        printf("\033[0m");
        break;
    case THREAD:
        printf("\033[0;3%dm", num);
        printf("<THREAD #%d> %s", num, message);
        printf("\033[0m");
        break;
    case THREAD_ERROR:
        printf("\033[1;3%dm", num);
        printf("<THREAD №%d>\033[0m\033[1;31m %s", num, message);
        printf("\033[0m");
        break;
    }
    if (GetResponse) {
        scanf("%s", message);
        printf("%s", message);
    }
}
