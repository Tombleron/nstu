#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 256

int main(int argc, char* argv[]) {

		HANDLE hNamedPipe;

		DWORD cbWritten, cbRead;
		char buffer[BUF_SIZE], pipeName[] = "\\\\.\\pipe\\TestPipe";

		STARTUPINFO si[100];
		PROCESS_INFORMATION pi[100];

		DWORD finish;


		if (argc < 3) {
				printf("There must be at least two arguments\n");
				return -1;
		}



		int i = 0;
		for (i = 0; i < (argc - 2); i++) {
				ZeroMemory(&si[i], sizeof(si[i]));
				si[i].cb = sizeof(si);
				ZeroMemory(&pi[i], sizeof(pi[i]));

				if (CreateProcess(NULL, "a.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si[i], &pi[i])) {
						printf("Process %lu started for file\n", pi[i].dwProcessId);
				} else {
						printf("CreateProcess failed. error: %Lu\n", GetLastError());
						return -2;
				}
		}


		int done = 0, jobs = (argc - 2), changes = 0;
		hNamedPipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX,
						PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
						PIPE_UNLIMITED_INSTANCES, BUF_SIZE, BUF_SIZE, 0, NULL);
		if (hNamedPipe == INVALID_HANDLE_VALUE) {
				printf("Failed to create pipe\n");
				return -3;
		}

		while (done != argc - 2) {
				if (!ConnectNamedPipe(hNamedPipe, NULL)) {
						printf("Error connectiong to a pipe\n");
						return -4;
				}

				if (ReadFile(hNamedPipe, buffer, BUF_SIZE, &cbRead, NULL)) {
						printf("Received command: %c\n", buffer[0]);
						switch (buffer[0]) {
								case 'n':
										WriteFile(hNamedPipe, 
														argv[argc - jobs], 
														strlen(argv[argc - jobs]) + 1, 
														&cbWritten, 
														NULL);
										jobs--;
										break;
								case 'c':
										WriteFile(hNamedPipe, 
														argv[1], 
														strlen(argv[1]) + 1, 
														&cbWritten, NULL);
										break;
								case 'p':
										WriteFile(hNamedPipe, 
														"p", 
														2, 
														&cbWritten, NULL);
										changes += atoi(buffer + 1);
										done++;
										break;
								default:
										printf("Unknown comman\n");
										break;
						}
				} else {
						printf("Error reading pipe\n");
				}
				
				DisconnectNamedPipe(hNamedPipe);		
		}

		for (i = 0; i < (argc - 2); i++) {
				finish = WaitForSingleObject(pi[i].hProcess, INFINITE);
				if (finish == WAIT_OBJECT_0) {
						printf("Process %Lu finished his work\n", pi[i].dwProcessId);
				} else {
						printf("Process %Lu failed his job\n", pi[i].dwProcessId);
						CloseHandle(pi[i].hProcess);
						CloseHandle(pi[i].hThread);
						TerminateProcess(pi[i].hProcess, 0);
				}
		}
		printf("Total changes: %i\n", changes);
		return 0;
}
