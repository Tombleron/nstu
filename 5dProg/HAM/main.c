#include "stdio.h"
#include "math.h"

#define MAX_VALUE 2048

void print_binary(unsigned short num) {
	for (int i = 14; i >= 0; i--) {
		int temp = num >> i;
		if (temp & 1) {
			printf("1");
		} else {
			printf("0");
		}
	}
}

unsigned short pop_count(unsigned short num) {

	int ret = 0;
	while (num != 0) {
		if (num & 1) ret += 1;
		num >>= 1;
	}
	return ret;

}

unsigned short encode_num(unsigned short num) {

	unsigned short ret = 0;

	unsigned short first = 0b1;
	unsigned short second = 0b10;
	unsigned short third = 0b1000;
	unsigned short fourth = 0b10000000;

	for (unsigned short i = 1; i <= 15; i++) {
		if (pop_count(i) != 1) {
			if (num & 1) {
				ret += 1 << (i-1);
				if (i & 0b1) ret ^= first;
				if (i & 0b10) ret ^= second;
				if (i & 0b100) ret ^= third;
				if (i & 0b1000) ret ^= fourth;
			}
			num >>= 1;
		}
	}

	return ret;
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

unsigned short fix(unsigned short num) {

	unsigned short first = 0b1;
	unsigned short second = 0b10;
	unsigned short third = 0b1000;
	unsigned short fourth = 0b10000000;

	unsigned short first_c = 0;
	unsigned short second_c = 0;
	unsigned short third_c = 0;
	unsigned short fourth_c = 0;

	for (unsigned short i = 1; i <= 15; i++) {
		if (pop_count(i) != 1) {
			if (num & (1<<(i-1))) {
				if (i & 0b1) first_c ^= 1;
				if (i & 0b10) second_c ^= 1;
				if (i & 0b100) third_c ^= 1;
				if (i & 0b1000) fourth_c ^= 1;
			}
		}
	}

	unsigned short pos = 0;

	if (first_c != ((num & first) > 0)) {
		pos += 0b1;
	}
	if (second_c != ((num & second) > 0)) {
		pos += 0b10;
	}
	if (third_c != ((num & third) > 0)) {
		pos += 0b100;
	}
	if (fourth_c != ((num & fourth) > 0)) {
		pos += 0b1000;
	}

	if (pos) {

		num ^= 1 << (pos - 1);

	}


	return num;

}

unsigned short from(unsigned short num) {

	unsigned short ret = 0;

	unsigned short first = 0b1;
	unsigned short second = 0b10;
	unsigned short third = 0b1000;
	unsigned short fourth = 0b10000000;

	unsigned short mask = 1;

	int c = 0;
	for (unsigned short i = 1; i <= 15; i++) {
		if (pop_count(i) != 1) {
			ret += ((num >> i-1) & 1) << c; 
			c++;
		}
		mask <<= 1;
	}

	return ret;


}

void decode() {

	char c;
	int res = 0;
	printf("Ведите число для декодирования: ");
	for (int i = 0; i < 15; i++) {
		res <<= 1;
		scanf(" %c", &c);
		if (c == '1') {
			res += 1;
		}
	}


	printf("Result of fix: ");
	print_binary(fix(res));
	printf("\nResult of decoding: %d", from(fix(res)));

}



int main() {
	encode();
	printf("\n");
	decode();
}
