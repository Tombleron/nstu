#define MAX_ENTRIES 200
#define MAX_INPUT 100
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 


struct Entry {
	char name[30];
	int  year;
	int  salary;
};

int get_args_count(char *str) {
	int counter = 0;
	while (*str != '\0') {
		if (*str == ' ') {
			counter++;
		}
		str++;
	}
	return counter;
}	

int is_number(char *i) {
	if (*i >= '0' && *i <= '9') {
		return TRUE;
	} else {
		return FALSE;
	}
}

int parse_arguments(struct Entry *entry, char *str) {
	char *ptr;
	char name[30];
	ptr = &name;
	while (*str != ' ') {
		*ptr = *str;
		ptr++;
		str++;
	}
	str++;

	int year;
	char str_year[4];
	ptr = &str_year;
	while (*str != ' ') {
		if (is_number(str)) {
			*ptr = *str;
			ptr++;
			str++;
		} else {
			printf("Второй аргумент должен быть числом\n");
			return FALSE;
		}
	}
	year = atoi(str_year);
	str++;

	int salary;
	char str_salary[10];
	ptr = &str_salary;
	while (*str != '\n' && *str != '\0' && *str != ' ') {
		if (is_number(str)) {
			*ptr = *str;
			ptr++;
			str++;
		} else {
			printf("Третий аргумент должен быть числом\n");
			return FALSE;
		}
	}
	salary = atoi(str_salary);

	strcpy((*entry).name, name);
	(*entry).year = year;
	(*entry).salary = salary;
	return TRUE;
}



int open_db(struct Entry *db, char filename[]) {
	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Ошибка открытия файла\n");
		return 0;
	}

	int counter = 0;
	char line[MAX_INPUT];

	while (fgets(line, 100, fp) && counter < MAX_ENTRIES) {
		if (get_args_count(line) != 2) {
			printf("Ошибка парсинга, неверное кол-во элементов\n");
			return counter;
		}
		parse_arguments(&db[counter], line);
		counter++;
			
	}
	fclose(fp);
	printf("База данных \"%s\" открыта, количество элементов: %d\n", filename, counter);
	return counter;
}

void save_db(struct Entry db[], char filename[], int values_count) {
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Ошибка открытия файла\n");
		return;
	}
	for (int i = 0; i < values_count; i++) {
		char year[5];
		sprintf(year, "%d", db[i].year);
	       	char salary[11];
		sprintf(salary, "%d", db[i].salary);

		fputs(db[i].name, fp);
		fputc(' ', fp);
		fputs(year, fp);
		fputc(' ', fp);
		fputs(salary, fp);
		fputc('\n', fp);
	}
	fclose(fp);
	printf("База данных сохранена");
}

int add_entry(struct Entry db[], struct Entry *entry, int index, int values_count) {
	if (index > values_count || index == MAX_ENTRIES) {
		printf("Ошибка добавления записи, неверный индекс\n");
		return values_count;
	}
	db[index] = *entry;
	if (index == values_count) {
		return values_count + 1;
	} else {
		return values_count;
	}
}

void sort(struct Entry db[], int first, int last) {
	if (first < last) {
		int left = first, right = last, middle = db[(left + right) / 2].year;
		do {
			while  (db[left].year > middle) left++;
			while (db[right].year < middle) right--;
			if (left >= right) {
				struct Entry tmp = db[left];
				db[left] = db[right];
				db[right] = tmp;
				left++;
				right--;
			}
		} while (left <= right);
		sort(db, first, right);
		sort(db, left, last);
	}
}

void show(struct Entry db[], int index, int values_count) {
	if (index > values_count || index < 0) { 
		printf("Ошибка вывода, неверно указан индекс\n");
		return;
	}
	printf("┌───┬──────────────────────────────┬────┬──────────┐\n");
	printf("│%-3d│%-30s│%-4d│%10d│\n", 
		index, db[index].name, db[index].year, db[index].salary);
	printf("└───┴──────────────────────────────┴────┴──────────┘\n");
}


void show_all(struct Entry db[], int values_count) {
	printf("┌───┬──────────────────────────────┬────┬──────────┐\n");
	for (int i = 0; i < values_count; i++) {
		printf("│%-3d│%-30s│%-4d│%10d│\n",
			i, db[i].name, db[i].year, db[i].salary);
	}
	printf("└───┴──────────────────────────────┴────┴──────────┘\n");
}
	
int check_saved(int status) {
	if (status == FALSE) {
		printf("Есть несохраненные изменения, вы уверены, что хотите продолжить?(y/n)\n");
		char input[2];
		scanf("%1s", input);
		if  (strcmp(input, "y") == 0) { 
			return TRUE; 
		} else { 
			return FALSE;
		}
	} else { 
		return TRUE; 
	}
}

void main() {
	struct Entry db[MAX_ENTRIES];
	int values_count = 0;

	int running = TRUE;
	int saved = TRUE;

	char input[50];
	int num = 0;
	char help[] = "load [filename] - открытие базы данных\n"
		       "display - отображение всей таблицы\n"
		       "get [index] - отображение записи по индексу\n"
		       "add [index] [name year salary] - добавить или заменить запись по индексу\n"
		       "sort - отслотировать таблицу\n"
		       "save [filename] - сохранить базу данных в файл\n"
		       "exit - выход из базы данных\n"
		       "help - вывод данного сообщения\n";
	printf("%s&: ", help);

	while (running) {
		scanf("%50s", input);
		if (strcmp(input, "exit") == 0) {
			if ( check_saved(saved) == TRUE) {
				break;
			}
		} else if (strcmp(input, "load") == 0) {
			scanf("%s", input);
			if (check_saved(saved) == TRUE) {
				values_count = open_db(db, input);
			}
		} else if (strcmp(input, "display") == 0) {
			show_all(db, values_count);
		} else if (strcmp(input, "get") == 0) {
			scanf("%d", &num);
			show(db, num, values_count);
		} else if (strcmp(input, "save") == 0) {
			scanf("%s", input);
			save_db(db, input, values_count);
			saved = TRUE;
		} else if (strcmp(input, "add") == 0) {
			scanf("%d", &num);
			scanf(" %[^\n]", input);
			struct Entry new;
			if (parse_arguments(&new, input) == TRUE) {
				values_count = add_entry(db, &new, num, values_count);
				saved = FALSE;
			}
		} else if (strcmp(input, "sort") == 0) {
			sort(db, 0, values_count-1);
			printf("База данных отсортирована\n");
			saved = FALSE;
		} else if (strcmp(input, "help") == 0) {
			printf("%s", help);
		}
		printf("&: ");
	}	
}
