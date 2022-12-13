#include "../helpers/file_actions.h"
#include "../helpers/logging.h"

#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 256

// Глобальный мьютекс (сразу инициализируем)
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int sock;

typedef struct {
  char content[BUFFER_SIZE];
  char symbol;
} Message;

typedef struct {
  Message msg;
  int thread_id;
  struct sockaddr_in client_addr;
} Arg;

void send_message(Arg *arg) {
  pthread_mutex_lock(&mutex);
  INFO("Sending message to client.")
  if (sendto(sock, &arg->msg, sizeof(Message), 0,
             (struct sockaddr *)&arg->client_addr,
             sizeof(struct sockaddr_in)) < 0) {
    ERROR("Cannot send message to client.");
  }
  pthread_mutex_unlock(&mutex);
}

void *client_handler(void *a) {
  // Копируем указатель в качестве указателя на структуру сообщения
  Arg *arg = a;
  Message *msg = &arg->msg;
  // Дескрипторы файлов
  int inputFile, outputFile;
  // Имя выходного файла (или путь к нему)
  char outputFileName[BUFFER_SIZE], inputFileName[BUFFER_SIZE];
  strcpy(inputFileName, msg->content);

  // Открытие входного файла
  inputFile = open(msg->content, O_RDONLY);
  if (inputFile == -1) {
    ERROR("<THREAD #%d>  Cannot open input file \"%s\": %s.\n", arg->thread_id,
          msg->content, strerror(errno));
    sprintf(msg->content, "Cannot open input file \"%.*s\": %s.\n", 100,
            inputFileName, strerror(errno));
    send_message(arg);
    free(arg);
    return NULL;
  }

  // Находим имя выходного файла
  strcpy(outputFileName, msg->content);
  strcat(outputFileName, ".modified");

  // Создаем файл, если его нет, с правами rx-rx-rx-
  outputFile = open(outputFileName, O_CREAT | O_WRONLY | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (outputFile == -1) {
    ERROR("<THREAD #%d>  Cannot open output file \"%s\": %s.\n", arg->thread_id,
          outputFileName, strerror(errno));
    sprintf(msg->content, "Cannot open output file \"%.*s\": %s.\n", 100,
            outputFileName, strerror(errno));
    send_message(arg);
    free(arg);
    return NULL;
  }

  // Обработка файла
  int result = process_file(inputFile, outputFile, msg->symbol);

  // Закрываем входной файл
  if (close(inputFile) == -1) {
    ERROR("<THREAD #%d>  Cannot close input file \"%s\": %s.\n", arg->thread_id,
          msg->content, strerror(errno));
    sprintf(msg->content, "Cannot close input file \"%.*s\": %s.\n", 100,
            inputFileName, strerror(errno));
    send_message(arg);
    free(arg);
    return NULL;
  }

  // Закрываем выходной файл
  if (close(outputFile) == -1) {
    ERROR("<THREAD #%d> Cannot close output file \"%s\": %s.\n", arg->thread_id,
          outputFileName, strerror(errno));
    sprintf(msg->content, "Cannot close output file \"%.*s\": %s.\n", 100,
            outputFileName, strerror(errno));
    send_message(arg);
    free(arg);
    return NULL;
  }

  sprintf(msg->content, "%d changes in file", result);
  send_message(arg);

  INFO("<THREAD #%d> done.", arg->thread_id);

  free(arg);
  return NULL;
}

int main(int argc, char **argv) {

  DBG("argc = %d", argc);

  for (int i = 0; i < argc; i++) {
    DBG("argv[%d] = %s", i, argv[i]);
  }

  if (argc < 2) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <port>\n");
    exit(-1);
  }

  // Создание сокета
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    ERROR("Cannot create socket.");
    exit(-2);
  }

  // Set non blocking
  fcntl(sock, F_SETFL, O_NDELAY);

  struct sockaddr_in server_addr, client_addr;
  // Создание адреса
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[1]));
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in))) {
    ERROR("Failed to bind the socker");
    exit(-3);
  }

  INFO("Waiting for clients...");

  Message msg;
  uint size = sizeof(struct sockaddr_in);
  int threads = 0;

  for (;;) {
    // Ожидание сообщения от клиента
    pthread_mutex_lock(&mutex);
    if (recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr,
                 &size) < 0) {
      if (errno == EWOULDBLOCK) {
        pthread_mutex_unlock(&mutex);
        continue;
      }
      ERROR("Cannot receive client's message. %s", strerror(errno));
      exit(-4);
    }
    pthread_mutex_unlock(&mutex);

    // Если пришло сообщение о завершении работы
    if (!strcmp(msg.content, "shutdown")){
      INFO("Shuting down.")
      break;
    }

    INFO("Received message from [%s]:\n\tFile: <%s>, Symbol: <%c>.",
         inet_ntoa(client_addr.sin_addr), msg.content, msg.symbol);

    pthread_t i;
    Arg *arg = malloc(sizeof(Arg));
    memcpy(&arg->msg, &msg, sizeof(Message));
    arg->thread_id = threads;
    arg->client_addr = client_addr;

    pthread_create(&i, NULL, client_handler, arg);
    threads++;
  }
}
