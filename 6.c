#include <stdio.h>
#include <string.h>





int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Error");
		return 0;
	}
	printf("%c", argv[2]);
	FILE *fp;
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Error");
		return 0;
	}
	FILE *fp2;
	fp2 = fopen(strcat(argv[1], ".out"), "w");
	char line[50];
	while (fgets(line, 50, fp)) {
		if (strncmp(line, argv[2], 2) == 0) {
			fputs(line, fp2);
		}
	}
	fclose(fp);
	fclose(fp2);

}
	
