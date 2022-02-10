#include "stdio.h"
#include "math.h"

#define MAX_VALUE 70

unsigned char allowed[56];

void print_binary(unsigned char num) {
	for (int i = 7; i >= 0; i--) {
		int temp = num >> i;
		if (temp & 1) {
			printf("1");
		} else {
			printf("0");
		}
	}
}

int pop_count(unsigned char num) {

	int ret = 0;
	while (num != 0) {
		if (num & 1) ret += 1;
		num >>= 1;
	}
	return ret;

}

void generate_allowed() {
	int index = 0;
	for (unsigned char i = 0; i <= 128; i++) {
		if (pop_count(i) == 3) {
			allowed[index] = i;
			index++;
		}
	}
}


unsigned char encode_num(unsigned char num) {
	return allowed[num];
}

void encode() {

	int input;
	printf("Ведите число для кодирования: ");
	scanf("%d", &input);
	if (input >= MAX_VALUE) {
		printf("Данный код не может закодировать это число.\n[MAX_VALUE == %d]", MAX_VALUE);
		return;
	}

	printf("Результат кодирования: ");
	print_binary(encode_num(input));

}

unsigned char decode_num(unsigned char num) {

	for (int i = 0; i < 56; i++) {
		if (allowed[i] == num) {
			return i;
		}
	}

}

void decode() {

	char c;
	int res = 0;
	int count = 0;
	printf("Ведите число для декодирования: ");
	for (int i = 0; i < 8; i++) {
		res <<= 1;
		scanf(" %c", &c);
		if (c == '1') {
			count++;
			res += 1;
		}
	}

	if (count != 3) {
		printf("Код содержит ошибку.\n");
		return;
	}


	printf("Result is %d\n", decode_num(res));

}

int main() {
	
	generate_allowed();
	printf("1. Закодировать\n2. Декодировать\n");
	char c;
	scanf("%c", &c);
	if (c == '1') {
	encode();
	} else {
	decode();
	}
}
