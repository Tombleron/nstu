#include "stdio.h"
#include "math.h"
#include "stdlib.h"

typedef struct {
	char nibble : 4;
	int real;
} BCDChar;

typedef struct {
	BCDChar* string;
	unsigned int length;
	int weights[4];
	BCDChar allowed[10];
} BCDString;

void print_binary(int num) {
	for (int i = 3; i >= 0; i--) {
		int temp = num >> i;
		if (temp & 1)
			printf("1");
		else
			printf("0");
	}
}

int weight(int* weights, int num) {
	unsigned int real = 0;
	for (int i = 3; i >= 0; i--) {
		unsigned int temp = num >> i;
		if (temp & 1)
			real += weights[3-i];
	}
	return real;
}

void get_allowed(int* weights, BCDChar* b) {

	int a[10] = {0};

	int i = 0;

	for (int j = 0; j < 16; j++) {
		int temp = weight(weights, j);
		if (temp < 10 && !a[temp]) {
			BCDChar num = { (char)j, temp };
			b[temp] = num;
			a[temp] = 1;
			i++;
		}
	}
}

int is_in_allowed(BCDChar* b, char a) {

	for (int i = 0; i < 10; i++) {

		if ((a & 15) == (b[i].nibble & 15)) {
			return i;
		}

	}

	return -1;
}

void parse_weights(int* weights) {

	for (int i = 0; i < 4; i++) {
		printf("Enter %d weight: ", i+1);
		scanf("%d", weights+i);
	}

}


int find_k(double M) {
	return (int)ceil(log10(M)) * 4;
}

int hamming_dist(char num1, char num2) {
	int dist = 0;
	for (int i = 0; i < 4; i++) {
		if (num1 % 2 != num2 % 2) {
			dist++;
		}
		num1 >>= 1;
		num2 >>= 1;
	}
	return dist;
}

int find_d(BCDString s) {
	int d = 4;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			if (j != i) {
				int dist=hamming_dist(s.allowed[i].nibble, s.allowed[j].nibble);
				if (d > dist) {
					d = dist;
				}
			}
		}
	}
	return d;
}

double find_D(int k) {
	return 1.0 - ((double)k/log2f((double)pow(10, k)));
}

void encode(BCDString string) {

	printf("Enter number to encode: ");
	int number;
	scanf("%d", &number);
	if (number > pow(10, string.length)-1) {
		printf("Error");
	} else {

		for (int i = string.length-1; i >= 0; i--) {

			int remainder = number % 10;
			number /= 10;
			BCDChar new_bcd;
			new_bcd.nibble = (char)string.allowed[remainder].nibble;
			new_bcd.real = number;
			string.string[i] = new_bcd;

		}

		printf("Encoded number: ");
		for (int i = 0; i < string.length; i++) {
			print_binary((int)string.string[i].nibble);
			printf(" ");
		}

	}
}

void decode(BCDString string) {

	int result = 0;
	printf("Enter %d binary numbers: ", string.length);
	for (int i = 0; i < string.length; i++) {
		char nibble = 0;
		for (int j = 0; j < 4; j++) {
			char input;
			scanf(" %c", &input);
			if (input == '1') {
				nibble += (1 << (3-j));
			}

		}

		int check = is_in_allowed(string.allowed, nibble);
		if ( check >= 0) {
			result += check * pow(10, string.length-i-1);
		} else {
			printf("Unallowed sequence!\nExiting.");
			break;
		}
	}
	printf("Result is %d", result);

}

int main() {


	BCDString string;

	int M;
	printf("Enter M: ");
	scanf("%d", &M);

	string.length = find_k(M) / 4;
	string.string = malloc(sizeof(BCDChar) * string.length);

	parse_weights(string.weights);

	printf("Allowed combinations are:\n");
	get_allowed(string.weights, string.allowed);

	for (int i = 0; i<10; i++) {
		printf("=> ");
		print_binary((int)string.allowed[i].nibble);
		printf(" -> %d\n", string.allowed[i].real);
	}

	printf("=> n is %d\n", string.length*4);
	printf("=> k is %d\n", string.length*4);
	printf("=> d is %d\n", find_d(string));
	printf("=> D is %f\n", find_D(string.length));

	printf("Do you want to [E]ncode or to [D]ecode? ");
	char input;
	scanf(" %c", &input);

	switch (input) {
		case 'E':
			encode(string);
			break;
		case 'D':
			decode(string);
			break;
		default:
			printf("NO");

	}
}
