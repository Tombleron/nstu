#include <stdio.h>
#include <string.h>
#include <windows.h>

#define MAX_THREADS 8
#define BUF_SIZE 256
#define LIB_NAME "dlltest.dll"

typedef struct MyData {
    int num;
    char file[80];
    int spaces;
} MYDATA, *PMYDATA;

typedef enum MessageType {
  SERVER,
  SERVER_ERROR,
  THREAD,
  THREAD_ERROR,
} MessageType;

void DisplayMessage(MessageType type, char* message, int num, BOOL GetResponse);
DWORD WINAPI MyThreadFunction(LPVOID lpParam);

int main(int argc, char *argv[]) {
    MYDATA pDataArray[MAX_THREADS];
    DWORD dwThreadIdArray[MAX_THREADS];
    HANDLE hThreadArray[MAX_THREADS];
    char message[BUF_SIZE] = "Not enough arguments. There must be at least 3 of them. Usage: ./thread.exe [changes] [filenames]";

    if (argc < 3) {
        DisplayMessage(SERVER_ERROR, message, 0, TRUE);
        exit(-1);
    }

    for (int i = 2; i < argc; i++) {

        pDataArray[0].num = i - 2;
        pDataArray[i - 2].num = i - 2;
        strcpy(pDataArray[i - 2].file, argv[i]);
        pDataArray[i - 2].file[strlen(argv[i])] = '\0';
        pDataArray[i - 2].spaces = 0;

        // Create the thread to begin execution on its own.

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
            DisplayMessage(SERVER_ERROR, "anime", 0, FALSE);
            ExitProcess(3);
        }
    } // End of main thread creation loop.

    // Wait until all threads have terminated.
    printf("\n");

    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

    // Close all thread handles and free memory allocations.

    // for (int i = 0; i < MAX_THREADS; i++) {
    //    CloseHandle(hThreadArray[i]);
    //    if (pDataArray[i] != NULL) {
    //        HeapFree(GetProcessHeap(), 0, pDataArray[i]);
    //        pDataArray[i] = NULL; // Ensure address is not reused.
    //    }
    //}

    return 0;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {

    PMYDATA pDataArray;
    char message[BUF_SIZE];

    // Cast the parameter to the correct data type.
    // The pointer is known to be valid because
    // it was checked for NULL before the thread was created.
    pDataArray = (PMYDATA)lpParam;

    // Print the parameter values. 
    sprintf(message, "Parameters = %s, %d\n", pDataArray->file, pDataArray->spaces);
    DisplayMessage(THREAD, message, pDataArray->num, FALSE);

    return 0;
}

void DisplayMessage(MessageType type, char* message, int num, BOOL GetResponse) {
  switch (type) {
    case SERVER:
        printf("<SERVER> %s", message);
        break;
    case SERVER_ERROR:
        printf("\x1b[31m");
        printf("<SERVER> [Error: %lu] %s", GetLastError(), message);
        printf("\x1b[0m");
        break;
    case THREAD:
        printf("<THREAD №%d> %s", num, message);
        break;
    case THREAD_ERROR:
        printf("\033[0;31m");
        printf("<THREAD №%d> %s", num, message);
        printf("\033[0m");
        break;
  }
  if (GetResponse) {
    scanf("%s", message);
    printf("%s", message);
  }
}
