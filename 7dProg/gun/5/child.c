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
#define SERVER_FLAGS O_RDONLY
// Флаги для открытия очереди клиента
#define CLIENT_FLAGS O_WRONLY | O_CREAT

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <character> [filename]\n");
    exit(-1);
  }

  Message msg;
  mqd_t server_mq;

  // Количество считанных байт данных
  int n = 0;
  // Атрибуты очереди сообщений
  struct mq_attr attr;
  // Ответ от сервера
  void *server_msg;

  // Открываем очередь сообщений клиента с атрибутами по умолчанию
  mqd_t client_mq = mq_open(CLIENT_MQ, CLIENT_FLAGS, FILE_MODE, NULL);
  if (client_mq < 0) {
    ERROR("Cannot open client's message queue.");
    return -1;
  }

  // Открываем очередь сообщений сервера
  INFO("Wait for the server message queue to appear...");
  while ((server_mq = mq_open(SERVER_MQ, SERVER_FLAGS)) < 0)
    sleep(1);
  INFO("OK.");
  // Получаем атрибуты очереди сообщений сервера
  mq_getattr(server_mq, &attr);
  // Выделяем память под ответ сервера
  server_msg = malloc(attr.mq_msgsize);

  sprintf(msg.content, "%s", argv[2]);

  // Получаем реальный символ
  msg.symbol = argv[1][0];

  // Отправляем сообщение серверу без приоритета
  n = mq_send(client_mq, (const char *)&msg, sizeof(Message), 0);

  // Проверка
  if (n == 0) {
    INFO("Message sent successfully.");
  } else {
    ERROR("Cannot sent message to the server: %s.", strerror(errno));
  }

  // // Если необходимо завершить работу
  // if (!strcmp(msg.content, "shutdown"))
  //   break;

  // Ожидаем ответа от сервера
  n = mq_receive(server_mq, server_msg, attr.mq_msgsize, NULL);
  INFO("Answer from the server: ");
  if (n > 0)
    printf("\t\t\t%s\n", (char *)server_msg);
  else if (n == 0)
    printf("<|empty message|>\n");
  else
    ERROR("%s", strerror(errno));

  // Высвобождаем память под сообщение
  free(server_msg);
  // Закрываем дескрипторы очередей
  mq_close(client_mq);
  mq_close(server_mq);

  exit(0);
}
