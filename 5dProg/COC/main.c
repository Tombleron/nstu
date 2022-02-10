#include "stdio.h"
#include "math.h"
#define K 8
#define SIZE 8

const int MAX_VALUE = pow(2, K);

typedef struct {
	unsigned char value[K/SIZE+(K%SIZE!=0)];
	unsigned char check;
} COC;

void print_binary(unsigned char num) {
	for (int i = SIZE; i >= 0; i--) {
		int temp = num >> i;
		if (temp & 1) {
			printf("1");
		} else {
			printf("0");
		}
	}
}

void display_COC(COC num) {
	for (int i = K-1; i>= 0; i--) {
		int index = i / SIZE;
		int bit = i - index*SIZE;
		int temp = num.value[index] >> bit;
		if (temp & 1) {
			printf("1");
		} else {
			printf("0");
		}
	}
	printf("%d", num.check);
}

COC into_COC(int num) {
	COC ret;
	ret.check = __builtin_popcount(num) & 1;
	int i = 0;
	while(num!=0) {
		int cropped = num & MAX_VALUE-1;
		ret.value[i] = cropped;
		num >>= SIZE;
		i++;
	}
	return ret;
}

int from_COC(COC num) {
	int ret = 0;

	for (int i = K/SIZE+(K%SIZE!=0)-1; i >= 0; i--) {
		ret = ret << SIZE;
		ret += num.value[i];
	}

	return ret;
}

int find_n(int k) {
	return k+1;
}

void encode() {

	int input;
	printf("Ведите число для кодирования: ");
	scanf("%d", &input);
	if (input > MAX_VALUE) {
		printf("Данный код не может закодировать это число.\n[MAX_VALUE == %d]", MAX_VALUE);
		return;
	}

	COC result = into_COC(input);
	printf("Результат кодирования:\t");
	display_COC(result);

}

void decode() {


	COC coc;
	unsigned char check = 0;
	char c;

	for (int i = SIZE-1; i >= 0; i--) {
		scanf(" %c", &c);
		unsigned char val = 0;
		if (c == '1') {
			val = 1;
			check++;
		}
		coc.value[i/SIZE] += val << i%SIZE;
		printf("%c", c);
	}

	scanf(" %c", &c);
	printf("%d %c", check, c);
	if (check % 2 != c-48) {
		printf("Incorrect code!\n");
		return;
	}

	printf("Decoded number is %d\n", from_COC(coc));

}

int main() {
	
	decode();
}
