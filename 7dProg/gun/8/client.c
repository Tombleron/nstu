#include "../helpers/file_actions.h"
#include "../helpers/logging.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 256

// Структура для обмена данными
typedef struct {
  char content[BUFFER_SIZE];
  char symbol;
} Message;

void exit_cleanup(int code, int sock) {
  close(sock);
  exit(code);
}

int main(int argc, char **argv) {
  // =============== ПЕРЕМЕННЫЕ СОКЕТА ===============
  // Дескриптор сокета
  int sock;
  // Структура адреса сервера
  struct sockaddr_in dest_addr;
  // Размер структуры адреса
  unsigned int structlen = sizeof(struct sockaddr_in);

  Message msg;

  if (argc < 4) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <port> <char> <filename>\n");
    exit(-1);
  }

  // Создание сокета
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    ERROR("Cannot create socket.");
    exit(-1);
  }

  // Создание адреса
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(atoi(argv[1]));
  dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Получаем имя файла
  strcpy(msg.content, argv[3]);
  msg.symbol = argv[2][0];

  // Попытка отправить пакет
  if (sendto(sock, &msg, sizeof(msg), 0, (const struct sockaddr *)&dest_addr,
             sizeof(dest_addr)) < 0) {
    ERROR("Cannot send message to the server.");
    exit_cleanup(-2, sock);
  }

  // Говорим, что всё ок
  INFO("Message sent successfully.");
  if (!strcmp(msg.content, "shutdown")) {
    exit(0);
  }

  // Говорим, что ждём сервер
  INFO("Wait for the server...\n");

  // Если пришёл ответ
  if (recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&dest_addr,
               &structlen) < 0) {
    ERROR("Cannot receive answer from the server.");
    exit_cleanup(-3, sock);
  }

  // Выводим на экран ответ сервера
  INFO("Answer from the server: \n\t%s.", msg.content);

  // Закрываем сокет
  close(sock);

  exit(0);
}
