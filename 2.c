#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define raz 5
#define razstr 100

/* Ввод строк */
int input(char string[], int maxlen) {
    int i, j;
    while (1) {
	    printf("Введи строку:");
	    gets(string);
	    j = strlen(string);
	    if (j <= maxlen) { break; }
	    printf("Длина строки должна быть <= %d", maxlen);
    }
    int k = 0;
    for (i = 0; i < j; i++) {
	    if (48 <= string[i] && string[i] <= 57) k++;
    }
    return(k);
}

/* Вывод строк */
void output(char string[], int length, int number) {
	number++;
	printf("Строка номер - %d\n%s\nKоличество цифр = %d\n", number, string, length);
	if (length < 2) {
		printf("Нет второго символа\n\n");
	} else {
		char k;
		int j = 0;
		int i = 0;

		while (j < 2) {
			if (48 <= string[i] && string[i] <= 57) {
				j++;
				k = string[i];
			}
			i++;
		}
		printf("Вторая цифра: %c\n\n", k);
	}
}

/* Замена строк */
void zamena(char str1[], char str2[]) {
	char temp[razstr];
	strcpy(temp, str1);
	strcpy(str1, str2);
	strcpy(str2, temp);
}

/* Сортировка строк */
int sort(char mas[][razstr], int len[], int r, int* min) {
	int i, j, k = 0,temp;

	*min = 0;
	for (i = 0; i < r - 1; i++) {
		for (j = 0; j < r - 1; j++) { 
			if (len[j] > len[j+1]) {
				temp = len[j];
				len[j] = len[j + 1];
				len[j + 1] = temp;
				k++;
				zamena(mas[j], mas[j + 1]);
			}
		}
	} 

	for (i = 0; i < r - 1; i++) {
		for (j = 0; j < r - 1; j++) {
			if (len[j] < len[j + 1]) {
				*min = len[j];
			}
		}
	}

	return (k);
}

/* Главная */
void main() {
	char mas[raz][razstr];
	int len[raz];
	int i, min, num;
	for (i = 0; i < raz; i++) {
		len[i] = input(mas[i], razstr);
	}
	num = sort(mas, len, raz,&min);
	for (i = 0; i < raz; i++) {
		output(mas[i], len[i], i);
	}

}

