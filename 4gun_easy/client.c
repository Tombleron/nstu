#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#define BUF_SIZE 256
#define MAX_TRIES 5000

int (*process_text)(char text[],
                    char output[],
                    int mod_count,
                    int mod_max,
                    int buf_size);

void generate_filenames(CHAR name[], CHAR file1[], CHAR file2[]) {
  strcpy(file1, name);
  strcpy(file2, name);
  strcat(file1, ".txt");
  strcat(file2, ".dxd");
}

int make_request(HANDLE hRead,
                 HANDLE hWrite,
                 DWORD* cbWritten,
                 DWORD* cbRead,
                 char request[],
                 char buf[]) {
  if (!WriteFile(hWrite, request, strlen(request) + 1, cbWritten, NULL)) {
    printf("a");
    fprintf(stderr, "Error sending signal\n");
    return 2;
  }

  if (ReadFile(hRead, buf, BUF_SIZE, cbRead, NULL)) {
    return 0;
  } else {
    fprintf(stderr, "Error receiving message\n");
    return 3;
  }
}

int main() {
  DWORD cbWritten = 0, cbRead = 0;
  HINSTANCE hLib;

  int rCount = 0;

  HANDLE hIn, hOut;
  DWORD dIn, dOut;
  HANDLE hRead = GetStdHandle(STD_INPUT_HANDLE),
         hWrite = GetStdHandle(STD_OUTPUT_HANDLE);

  CHAR buf[BUF_SIZE], outbuf[BUF_SIZE];
  CHAR inName[100], outName[100];

  if (make_request(hRead, hWrite, &cbWritten, &cbRead, "n", buf)) {
    return -1;
  } else {
    fprintf(stderr, "Got name: %s\n", buf);
  }

  generate_filenames((buf), inName, outName);
  memset(buf, '\0', sizeof(buf));

  if (make_request(hRead, hWrite, &cbWritten, &cbRead, "c", buf)) {
    return -1;
  } else {
    fprintf(stderr, "Got max changes: %s\n", buf);
  }

  rCount = atoi(buf);
  memset(buf, '\0', sizeof(buf));

  hIn = CreateFile(inName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hIn == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Can't open input file: %lu\n", GetLastError());
    return -4;
  }

  hOut = CreateFile(outName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL, NULL);
  if (hOut == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Can't open output file: %lu\n", GetLastError());
    return -5;
  }

  hLib = LoadLibrary("dlltest.dll");
  if (hLib == NULL) {
    fprintf(stderr, "Cannot load library");
    return -6;
  }
  process_text = (int (*)(char*, char*, int, int, int))GetProcAddress(
      hLib, "process_text");
  if (process_text == NULL) {
    fprintf(stderr, "process_text function not found");
    return -7;
  }

  int r = 0;
  while (ReadFile(hIn, buf, BUF_SIZE, &dIn, NULL) && dIn > 0) {
    r = (*process_text)(buf, outbuf, r, rCount, BUF_SIZE);
    WriteFile(hOut, outbuf, dIn - r, &dOut, NULL);
  }

  char output[BUF_SIZE];
  sprintf(output, "p%i", r);
  if (make_request(hRead, hWrite, &cbWritten, &cbRead, output, buf)) {
    return -1;
  } else {
    fprintf(stderr, "Finished with %i changes\n", r);
  }

  CloseHandle(hIn);
  CloseHandle(hOut);
  FreeLibrary(hLib);
  return 0;
}
