#include <stdio.h>
#define strings_count 5 // Указываем количество строк
#define strings_len 10   // Указываем максимальную длину строк
// Cобственная реализация strcpy
void copy(char *str1, char *str2) {
           while (*str1++ = *str2++);   // Стоило бы проверять соответствие длин строк,
 					  // но так как они одинаковой длины, 
       					  // то можно опустить
}

// Функция получения длины строки
int str_len(char *str) {
  	int i = 0;
 	while (*str != '\0') { // Шагаем по строке пока не наткнемся на \0
 		i++;
 		str++;
 	}
 	return i;	
}
// Функция получения количества символов
int get_num_count(char *str) {
 	int count = 0;
 	while (*str != '\0') {		 	    // Пока не наткнемся на \0 идем по строке
                  if (48 <= *str && *str <= 57) { // и увеличиваем счетчик, 
 						    // если код символа соответствует цифре
 		      count++;
                 }
                 str++;
	}
           return count;
}	
// Метод ввода строки
int input(char *str, int maxlen) {
     int i, j;
     char inp_str[maxlen];
     printf("Введите строку: ");
     scanf("%s", inp_str);
     j = str_len(inp_str);
    while (j >= maxlen) {       // Если длина введенной строки больше максимальной, 
                                             // то просим ввести еще раз
 		printf("Длина строки должна быть <= %d\n", maxlen);
 		printf("Введите строку: ");
 		scanf("%s", inp_str);
 		j = str_len(inp_str);
    }	
    copy(str, inp_str);
    int count = get_num_count(inp_str);
    return count;                   // Возвращаем количество цифр в строке



// Метод для перестановки двух строк в массиве
void string_replace(char *str1, char *str2) {
 	char heap[strings_len];
 	сopy(heap, str1);
 	copy(str1, str2);
 	copy(str2, heap);
}
// Метод перестановки двух чисел в массиве
void num_replace(int *nums, int fst, int snd) {
 	int tmp = nums[fst];
 	nums[fst] = nums[snd];
 	nums[snd] = tmp;
}
// Метод сортировки массива, используется алгоритм QuickSort 
void sort(char strs[strings_count][strings_len], int *nums, int first, int last) {
 	if (first < last) {
 	     int left = first;
 	     int right = last;
                int middle = nums[(left + right) / 2];
 	     do {
  		while (nums[left]  < middle) left++;
 		while (nums[right] > middle) right--;
 		if (left <= right) {
 		    string_replace(strs[left], strs[right]);
 		    num_replace(nums, left, right); // Так как мы сортируем массив с 
                          left++;  				// количеством цифр, то для того, 
 		    right--;				// чтобы отсортировать массив строк,
  							// достаточно вносить такие же
 						 	// изменения, т.к. соотносящиеся
 							// элементы имеют одинаковый индекс		}
	     } while (left <= right);
 	     sort(strs, nums, first, right);
 	     sort(strs, nums, left, last);	
 	}
}




// Метод вывода строки
void output(char *str, int num) {
        printf("Строка: %s\n", str);
        if (num >= 2) {                        // Вывод может изменится в зависимости от
  	  char *tmp = str;	    	  // количества цифр в строке
 	  char *tmp = str;		  // В случае, если цифр больше 1,
 	  int cnt = 0; 			  // то мы можем вывести вторую цифру
	  char snd_num;
  	  while (cnt != 2) {
 	         if (*tmp >= 48 && *tmp <= 57) { // Тут можно спокойно прибавлять к 
  		   snd_num = *tmp;	                     // указателю без проверки на конец
 		   cnt++;                                        // строки, потому что мы точно знаем,
 	         } 				          // что в строке как минимум две цифры
	         tmp++;
 	 }					
 	 printf("Количество цифр: %d\nВторая цифра: %c\n\n", num, snd_num);
        } else {		 		// Если же в строке меньше 2 цифр, то выводится 
 					// соответствующее сообщение
 	 printf("Количество цифр: %d\nВ строке меньше двух цифр\n\n", num);
        }
}
int main() {
     char str[strings_count][strings_len];             // Создаем двумерный массив для строк
     int nums_count[strings_count];      	          // Массив количества цифр в строке
     for (int i = 0; i < strings_count; i++) {
 	 nums_count[i] = input(str[i], strings_len); // Ввод строки
     }
     printf("\n");
     sort(str, nums_count, 0, strings_count-1);  	   // Сортировка массива
     for (int i = 0; i < strings_count; i++) {
     output(str[i], nums_count[i]);			   // Вывод строк
     }
} 
