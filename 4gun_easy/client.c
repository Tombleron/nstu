// Программа из конспекта "Системное программное обеспечение"
// Версия для Windows
// стр. ?? 
// дочерний процесс для анонимных каналов

#include   <windows.h> 
#include   <stdio.h> 
#include   <stdlib.h> 
#include   <string.h> 

int main(int argc, char* argv[])
{
	FILE *hdl;
	char *line;
	DWORD   total = 0;
	char filename[80] = { 0 };
	char message[80] = { 0 };
	char result[10] = { 0 };
	DWORD cbWritten;
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), "(Child)Get filename to count spaces ",36, &cbWritten, NULL);
	ReadFile(GetStdHandle(STD_INPUT_HANDLE), filename, 80, &cbWritten, NULL);
	filename[cbWritten-2] = 0;
	//WriteFile(GetStdHandle(STD_ERROR_HANDLE), filename, strlen(filename), &cbWritten, NULL);
	if (hdl = fopen(filename, "rt")){
		// цикл чтения до конца файла 
		while (!feof(hdl)) {
			// чтение одного символа из файла
			if ((char)fgetc(hdl) == 0x20) total++;
		}
		// сообщение в консоль ошибок 
		sprintf(message,"\n(Child): %Lu, file:%s, spaces = %d\n", GetCurrentProcessId(), filename, total);
		WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
		// сообщение в канал 
		sprintf(result, "%d", total);
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), result, strlen(result) + 1, &cbWritten, NULL);
		// закрытие файла
		fclose(hdl);
	}
	else {
		// сообщение в канал 
		sprintf(message, "(Child)Can't open %s!",filename);
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
		return (-1);
	}
	return total;
}