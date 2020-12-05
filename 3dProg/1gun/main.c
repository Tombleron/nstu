#include <windows.h> 
#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#define BUF_SIZE 256

void generate_filenames(CHAR name[], CHAR file1[], CHAR file2[]) {
		strcpy(file1, name);
		strcpy(file2, name);
		strcat(file1, ".txt");
		strcat(file2, ".dxd");
}

int main(int argc, LPTSTR argv[]) {
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
	
	int r = 0;
	while(ReadFile(hIn, buf, BUF_SIZE, &dIn, NULL) && dIn > 0) { 
		int i = 0;
		int k = 0;
		for (i = 0; i < strlen(buf); i++) {
			if (buf[i] == ' ' && r < rCount) { r++; continue;}
			else if (buf[i] == '\0') { outbuf[k] = '\0'; break; }
			else { outbuf[k] = buf[i]; k++; }
		}
		WriteFile (hOut, outbuf, dIn, &dOut, NULL); 
	}
	CloseHandle (hIn); 
	CloseHandle (hOut); 
	return 0;
} 

