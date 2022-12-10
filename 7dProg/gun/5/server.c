#include "../helpers/file_actions.h"
#include "../helpers/logging.h"
#include "defines.h"

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Флаги для открытия очереди сервера
#define SERVER_FLAGS O_WRONLY | O_CREAT | O_EXCL
// Флаги для открытия очереди клиента
#define CLIENT_FLAGS O_RDONLY

int send_message(mqd_t queue, char *buffer) {
  // Выводим на экран
  printf("%s.", buffer);
  // Отправляем сообщение клиенту
  if (mq_send(queue, buffer, strlen(buffer) + 1, 0) < 0) {
    ERROR("Cannot sent message to the client: %s.", strerror(errno));
    return -1;
  }
  return 0;
}

void exit_cleanup(int code) {
	if (mq_unlink(SERVER_MQ) < 0) 
		ERROR("Cannot remove message queue of server: %s.", strerror(errno));
	if (mq_unlink(CLIENT_MQ) < 0) 
		ERROR("Cannot remove message queue of clients: %s.", strerror(errno));
	exit(code);
}

int main(int argc, char **argv) {

  if (argc == 2 && !strcmp(argv[1], "c")) {
    exit_cleanup(0);
  }

  char buff[BUFFER_SIZE];

  DBG("Creating message queues");

  mqd_t client_mq;
  mqd_t server_mq = mq_open(SERVER_MQ, SERVER_FLAGS, FILE_MODE, NULL);
  if (server_mq < 0) {
    ERROR("Can't create server's queue: %s", strerror(errno));
    if (errno == EEXIST)
      INFO("\t\t\tMessage queue already exists");
    exit_cleanup(-1);
  }

  // Открываем очередь сообщений клиента
  INFO("Wait for the client message queue to appear...");
  // fflush(stdout);
  while ((client_mq = mq_open(CLIENT_MQ, CLIENT_FLAGS)) < 0)
    sleep(1);
  INFO("OK.");
  // Получаем атрибуты очереди сообщений клиента
  struct mq_attr attr;
  mq_getattr(client_mq, &attr);
  // Выделяем память под сообщение клиента
  // if (attr.mq_msgsize != sizeof(Message)) {
  //   DBG("Client message size: %ld", attr.mq_msgsize);
  //   ERROR("Message size incorrect.");
  //   exit(-10);
  // }
  Message *client_msg = malloc(attr.mq_msgsize);

  char out_filename[BUFFER_SIZE];

  while (1) {

    int n;
    // Чтение сообщения от клиента
    if ((n = mq_receive(client_mq, (void *)client_msg, attr.mq_msgsize, NULL)) <
        0) {
      ERROR("Cannot receive message: %s.", strerror(errno));
      continue;
    } else if (n == 0) {
      ERROR("Received message is empty.");
      continue;
    }

    // Читаем сообщение
    Message msg = *client_msg;
    // Если пришло сообщение о завершении работы
    if (!strcmp(msg.content, "shutdown")) {
      break;
    }

    // Выводим сообщение на экран
    INFO("Received message: File: <%s>, Symbol: <%c>.", msg.content,
         msg.symbol);

    // Открытие входного файла
    int input_file = open(msg.content, O_RDONLY);
    if (input_file == -1) {
      sprintf(buff, "[ERROR]: Cannot open input file \"%.*s\"", 219, msg.content);
      send_message(server_mq, buff);
      continue;
    }

    // Открытие выходного файла
    strcpy(out_filename, msg.content);
    strcat(out_filename, ".modified");
    // Создаем файл, если его нет, с правами rx-rx-rx-
    int out_file =
        open(out_filename, O_CREAT | O_WRONLY | O_TRUNC,
             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (out_file == -1) {
      sprintf(buff, "[ERROR]: Cannot open output file \"%.*s\"", 220, out_filename);
      send_message(server_mq, buff);
      continue;
    }

    // Обработка файла

    int result = process_file(input_file, out_file, msg.symbol);

    // Закрываем входной файл
    if (close(input_file) == -1) {
      sprintf(buff, "[ERROR]: Cannot close input file");
      send_message(server_mq, buff);
      continue;
    }

    // Закрываем выходной файл
    if (close(out_file) == -1) {
      sprintf(buff, "[ERROR]: Cannot close output file");
      send_message(server_mq, buff);
      continue;
    }

    // Отправляем ответ клиенту
    sprintf(buff, "Done. %d changes saved in %.*s", result, 221, out_filename);
    send_message(server_mq, buff);
  }
}
