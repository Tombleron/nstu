#include <string.h>

int process_text(char text[], char output[], int mod_count, int mod_max, int BUF_SIZE) {
		int i = 0;
		int k = 0;
		for (i = 0; i < strlen(text); i++) {
			if (text[i] == ' ' && mod_count < mod_max) { mod_count++; continue;}
			else if (text[i] == '\0') { output[k] = '\0'; break; }
			else { output[k] = text[i]; k++; }
		}
		return mod_count;
}
