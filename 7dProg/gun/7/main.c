#include "../helpers/file_actions.h"
#include "../helpers/logging.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 256

// Глобальный мьютекс (сразу инициализируем)
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  int thread_id;
  int result;
  char content[BUFFER_SIZE];
  char symbol;
} Message;

void *thread_func(void *arg) {
  // Блокируем мьютекс
  pthread_mutex_lock(&mutex);
  // Копируем указатель в качестве указателя на структуру сообщения
  Message *msg = arg;
  // Дескрипторы файлов
  int inputFile, outputFile;
  // Имя выходного файла (или путь к нему)
  char outputFileName[BUFFER_SIZE];
  // Открытие входного файла
  inputFile = open(msg->content, O_RDONLY);
  if (inputFile == -1) {
    ERROR("<THREAD #%d>  Cannot open input file \"%s\": %s.\n", msg->thread_id,
          msg->content, strerror(errno));
    // Разлокируем мьютекс
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  // Находим имя выходного файла
  strcpy(outputFileName, msg->content);
  strcat(outputFileName, ".modified");

  // Создаем файл, если его нет, с правами rx-rx-rx-
  outputFile = open(outputFileName, O_CREAT | O_WRONLY | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (outputFile == -1) {
    ERROR("<THREAD #%d>  Cannot open output file \"%s\": %s.\n", msg->thread_id,
          outputFileName, strerror(errno));
    // Разлокируем мьютекс
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  // Обработка файла
  msg->result = process_file(inputFile, outputFile, msg->symbol);

  // Закрываем входной файл
  if (close(inputFile) == -1) {
    ERROR("<THREAD #%d>  Cannot close input file \"%s\": %s.\n", msg->thread_id,
          msg->content, strerror(errno));
    // Разлокируем мьютекс
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  // Закрываем выходной файл
  if (close(outputFile) == -1) {
    ERROR("<THREAD #%d> Cannot close output file \"%s\": %s.\n", msg->thread_id,
          outputFileName, strerror(errno));
    // Разлокируем мьютекс
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  // Выводим радостное сообщение
  INFO("<THREAD #%d> done.", msg->thread_id);

  // Разлокируем мьютекс
  pthread_mutex_unlock(&mutex);

  // Возвращаем результат
  return &msg->result;
}

int main(int argc, char **argv) {

  DBG("argc = %d", argc);

  for (int i = 0; i < argc; i++) {
    DBG("argv[%d] = %s", i, argv[i]);
  }

  if (argc < 3) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <character> [filename]\n");
    exit(-1);
  }

  DBG("Allocation memory for threads and messages");

  pthread_t *threads = calloc(argc - 2, sizeof(pthread_t));

  Message *msgs = calloc(argc - 2, sizeof(Message));

  for (int i = 2; i < argc; i++) {

    DBG("Starting thread %d", i - 2);
    // Формируем сообщение для потока
    strcpy(msgs[i - 2].content, argv[i]); // Имя входного файла
    msgs[i - 2].symbol = argv[1][0]; // Символ для обработки
    msgs[i - 2].thread_id = i - 2; // Порядковый номер потока

    DBG(" content: %s\n\t symbol: %c\n\t id: %d", msgs[i - 2].content,
        msgs[i - 2].symbol, msgs[i - 2].thread_id);

    // Запускаем поток
    if (pthread_create(&threads[i - 2], NULL, thread_func, &msgs[i - 2]) != 0) {
      ERROR("Cannot create thread #%d.", i - 2);
      exit(-1);
    }
  }

  DBG("Waiting for results");

  void *result;
  // Запускаем цикл ожидания работы всех потоков
  for (int i = 0; i < (argc - 2); i++) {
    // Ожидаем завершения потока
    DBG("Waiting for thread <%d>", i);
    pthread_join(threads[i], &result);
    if (result == NULL) {
      ERROR("Thread #%d was unable to process the file \"%s\".", i,
            msgs[i].content);
    } else {
      // Выводим радостное сообщение
      INFO("Thread #%d done. %d changes written in output file "
           "\"%s.modified\".",
           i, *((int *)result), msgs[i].content);
    }
  }
}
