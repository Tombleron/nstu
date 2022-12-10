#include "../helpers/file_actions.h"
#include "../helpers/logging.h"
#include "defines.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int send_message(sem_t *sem, Message *msg, char *buffer) {
  // Выводим на экран
  INFO("%s.", buffer);
  // Очищаем память
  memset(msg, 0, sizeof(Message)+1);
  // Копируем строку
  strcpy(msg->content, buffer);
  // Отправляем сообщение клиенту
  if (sem_post(sem) < 0) {
    ERROR("Cannot sent message to the client: %s.", strerror(errno));
    return -1;
  }
  return 0;
}

void exit_cleanup(int code) {
  // Удаляем семафоры
  sem_unlink(SEM_CLIENT);
  sem_unlink(SEM_SERVER);
  // Удаляем сегмент разделяемой памяти
  shm_unlink(SHM);
  exit(code);
}

int main(int argc, char **argv) {

  if (argc == 2 && !strcmp(argv[1], "c")) {
    exit_cleanup(0);
  }

  char buff[BUFFER_SIZE];

  DBG("Creating semaphores");

  // Открываем семафор клиента, если не существует, создаём
  sem_t *sem_client = sem_open(SEM_CLIENT, SEM_CLIENT_FLAGS, SEM_FILE_MODE, 0);
  if (sem_client == SEM_FAILED) {
    ERROR("Cannot open client's semaphore: %s.", strerror(errno));
    exit_cleanup(-1);
  }

  // Открываем семафор сервера, если не существует, создаём
  sem_t *sem_server = sem_open(SEM_SERVER, SEM_SERVER_FLAGS, SEM_FILE_MODE, 0);
  if (sem_server == SEM_FAILED) {
    ERROR("Cannot open server's semaphore: %s.", strerror(errno));
    exit_cleanup(-1);
  }

  // Открываем или создаём разделяемую память
  int fd = shm_open(SHM, SHM_FLAGS, SHM_FILE_MODE);
  if (fd < 0) {
    ERROR("Cannot open shared memory: %s.", strerror(errno));
    exit_cleanup(-1);
  }
  // Обозначаем длину разделяемой памяти
  ftruncate(fd, sizeof(Message)+1);

  // Отображаем память в адресное пространство процесса
  Message *shm_ptr =
      mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_ptr == MAP_FAILED) {
    ERROR("Cannot map shared memory: %s.", strerror(errno));
    exit_cleanup(-1);
  }

  int out_file;
  char out_filename[BUFFER_SIZE];
  // Открываем очередь сообщений клиента
  INFO("Wait for the client message...");

  while (1) {
    // Ожидание сообщения от клиента
    if (sem_wait(sem_client) < 0) {
      ERROR("Cannot wait from client's semaphore: %s.", strerror(errno));
      continue;
    }

    // Если пришло сообщение о завершении работы
    if (!strcmp(shm_ptr->content, "shutdown"))
      break;

    // Выводим сообщение на экран
    INFO("Received message: File: <%s>, Symbol: <%c>.", shm_ptr->content,
           shm_ptr->symbol);

    // Открытие входного файла
    int input_file = open(shm_ptr->content, O_RDONLY);
    if (input_file == -1) {
      sprintf(buff, "[ERROR]: Cannot open input file \"%.*s\": %.*s", 100,
              shm_ptr->content, 100, strerror(errno));
      send_message(sem_server, shm_ptr, buff);
      continue;
    }

    // Находим имя выходного файла
    strcpy(out_filename, shm_ptr->content);
    strcat(out_filename, ".modified");

    // Создаем файл, если его нет, с правами rx-rx-rx-
    out_file = open(out_filename, O_CREAT | O_WRONLY | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (out_file == -1) {
      sprintf(buff, "[ERROR]: Cannot open output file \"%.*s\": %.*s", 100, out_filename, 100,
              strerror(errno));
      send_message(sem_server, shm_ptr, buff);
      continue;
    }

    // Обработка файла
    int result = process_file(input_file, out_file, shm_ptr->symbol);

    // Закрываем входной файл
    if (close(input_file) == -1) {
      sprintf(buff, "[ERROR]: Cannot close input file: %.*s", 200,strerror(errno));
      send_message(sem_server, shm_ptr, buff);
      continue;
    }

    // Закрываем выходной файл
    if (close(out_file) == -1) {
      sprintf(buff, "[ERROR]: Cannot close output file: %.*s", 200,strerror(errno));
      send_message(sem_server, shm_ptr, buff);
      continue;
    }

    // Отправляем ответ клиенту
    sprintf(buff, "Done. %d changes saved in %.*s", result, 200, out_filename);
    send_message(sem_server, shm_ptr, buff);
  }

  // Закрываем дескрипторы семафоров
  sem_close(sem_client);
  sem_close(sem_server);

  // Закрываем дескриптор разделяемой памяти
  close(fd);

  exit_cleanup(0);
}
