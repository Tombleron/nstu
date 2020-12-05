#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

		STARTUPINFO si[100];
		PROCESS_INFORMATION pi[100];
		char command[100];

		DWORD finish, result, total = 0;
		LPDWORD res = &result;

		if (argc < 3) {
				printf("There must be at least two arguments\n");
				return -1;
		}

		int i = 0;
		for (i = 0; i < (argc - 2); i++) {
				strcpy(command, "child.exe ");
				strcat(command, argv[i+1]);
				strcat(command, " ");
				strcat(command, argv[argc - 1]);
				printf("The command is: %s\n", command);

				ZeroMemory(&si[i], sizeof(si[i]));
				si[i].cb = sizeof(si);
				ZeroMemory(&pi[i], sizeof(pi[i]));

				if (CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si[i], &pi[i])) {
						printf("Process %lu started for file: %s\n", pi[i].dwProcessId, argv[i+1]);
						//Sleep(1000);
				} else {
						printf("CreateProcess failed. error: %Lu\n", GetLastError());
						return -2;
				}
		}

		for (i = 0; i < (argc - 2); i++) {
				finish = WaitForSingleObject(pi[i].hProcess, INFINITE);
				if (finish == WAIT_OBJECT_0) {
						printf("Process %Lu finished his work\n", pi[i].dwProcessId);
						GetExitCodeProcess(pi[i].hProcess, res);
						printf("Process %Lu made %d changes\n", pi[i].dwProcessId, result);
						total += result;
				} else {
						printf("Process %Lu failed his job\n", pi[i].dwProcessId);
						CloseHandle(pi[i].hProcess);
						CloseHandle(pi[i].hThread);
						TerminateProcess(pi[i].hProcess, 0);
				}
		}
		printf("%d files processed with total changes %d\n", argc-2, total);
		return 0;
}
