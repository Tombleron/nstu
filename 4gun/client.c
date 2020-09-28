#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256
#define MAX_TRIES 5000

int(*process_text)(char text[], char output[], int mod_count, int mod_max, int buf_size);

void generate_filenames(CHAR name[], CHAR file1[], CHAR file2[]) {
		strcpy(file1, name);
		strcpy(file2, name);
		strcat(file1, ".txt");
		strcat(file2, ".dxd");
}

int make_request(char pipeName[], DWORD *cbWritten, DWORD *cbRead, char request[], char buf[]) {
		HANDLE hNamedPipe;

		WaitNamedPipe(pipeName, NMPWAIT_WAIT_FOREVER);
		hNamedPipe = CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hNamedPipe == INVALID_HANDLE_VALUE) {
				printf("Failed to CreateFile on a pipe: %ld\n", GetLastError());
				return 1;
		} 

		if (!WriteFile(hNamedPipe, request, strlen(request) + 1, cbWritten, NULL)) {
				printf("Error sending signal\n");
				return 2;
		}

		if (ReadFile(hNamedPipe, buf, BUF_SIZE, cbRead, NULL)) {
				CloseHandle(hNamedPipe);
				return 0;
		} else {
				printf("Error receiving message\n");
				return 3;
		}
}


int main(int argc, char * argv[]) {

		DWORD cbWritten = 0, cbRead = 0;
		HINSTANCE hLib;

		char bufPipe[BUF_SIZE], pipeName[] = "\\\\.\\pipe\\TestPipe";
		int rCount = 0;

		HANDLE hIn, hOut;
		DWORD dIn, dOut;

		CHAR buf[BUF_SIZE], outbuf[BUF_SIZE];
		CHAR inName[100], outName[100];

		if (make_request(pipeName, &cbWritten, &cbRead, "n", buf)) {
				return -1;
		} else {
				printf("Got name: %s\n", buf);
		}

		generate_filenames((buf), inName, outName);
		memset(buf, '\0', sizeof(buf));

		if (make_request(pipeName, &cbWritten, &cbRead, "c", buf)) {
				return -1;
		} else {
				printf("Got max changes: %s\n", buf);
		}

		rCount = atoi(buf);
		memset(buf, '\0', sizeof(buf));

		hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hIn == INVALID_HANDLE_VALUE) {
				printf("Can't open input file: %x\n", GetLastError());
				return  -4;
		}

		hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOut == INVALID_HANDLE_VALUE) {
				printf("Can't open output file: %x\n", GetLastError());
				return -5;
		}

		hLib = LoadLibrary("dlltest.dll");
		if (hLib == NULL) {
				printf("Cannot load library");
				return -6;
		}
		process_text = (int(*)(char*, char*, int, int, int))GetProcAddress(hLib, "process_text");
		if (process_text == NULL) {
				printf("process_text function not found");
				return -7;
		}

		int r = 0;
		while(ReadFile(hIn, buf, BUF_SIZE, &dIn, NULL) && dIn > 0) {
				r = (*process_text)(buf, outbuf, r, rCount, BUF_SIZE);
				WriteFile(hOut, outbuf, dIn - r, &dOut, NULL);
		}

		char output[BUF_SIZE];
		sprintf(output, "p%i", &r);
		if (make_request(pipeName, &cbWritten, &cbRead, output, buf)) {
				return -1;
		} else {
				printf("Finished with %i changes\n", r);
		}

		CloseHandle (hIn);
		CloseHandle (hOut);
		FreeLibrary(hLib);
		return 0;
}


