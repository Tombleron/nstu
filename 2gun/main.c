#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256
HINSTANCE hLib;
int(*process_text)(char text[], char output[], int mod_count, int mod_max, int buf_size);

void generate_filenames(CHAR name[], CHAR file1[], CHAR file2[]) {
		strcpy(file1, name);
		strcpy(file2, name);
		strcat(file1, ".txt");
		strcat(file2, ".dxd");
}

int main(int argc, char * argv[])
{
	HANDLE hIn, hOut; 
	DWORD dIn, dOut;

	CHAR buf[BUF_SIZE], outbuf[BUF_SIZE]; 
	CHAR inName[100], outName[100];

	if(argc != 3) {
		printf("Usage: ./a.exe filename number\n"); 
		return  1; 
	} 

	int rCount = atoi(argv[2]);
	generate_filenames(argv[1], inName, outName);

	hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL); 
	if (hIn == INVALID_HANDLE_VALUE) {
		printf("Can't open input file: %x\n", GetLastError()); 
		return  2; 
	}

	hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
			printf("Can't open output file: %x\n", GetLastError());
			return 3;
	}
	
	hLib = LoadLibrary("dlltest.dll");
	if (hLib == NULL) {
		printf("Cannot load library");
		return -4;
	}
	process_text = (int(*)(char*, char*, int, int, int))GetProcAddress(hLib, "process_text");
	if (process_text == NULL) {
		printf("process_text function not found");
		return -5;
	}

	int r = 0;
	while(ReadFile(hIn, buf, BUF_SIZE, &dIn, NULL) && dIn > 0) {  
		r = (*process_text)(buf, outbuf, r, rCount, BUF_SIZE);
		WriteFile(hOut, outbuf, dIn - r, &dOut, NULL);
	}
	CloseHandle (hIn); 
	CloseHandle (hOut); 
	FreeLibrary(hLib);
	return 0;
}


