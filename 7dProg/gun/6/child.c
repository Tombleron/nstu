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


void exit_cleanup(int code, sem_t* sem_client, sem_t* sem_server, int fd) {
  
  	// Закрываем дескрипторы семафоров
	sem_close(sem_client);	
	sem_close(sem_server);

	// Закрываем дескриптор разделяемой памяти
	close(fd);

	exit(0);

}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <character> [filename]\n");
    exit(-1);
  }

  // Открываем семафор клиента, если не существует, создаём
  sem_t *sem_client = sem_open(SEM_CLIENT, SEM_CLIENT_FLAGS, SEM_FILE_MODE, 0);
  if (sem_client == SEM_FAILED) {
    ERROR("Cannot open client's semaphore: %s.", strerror(errno));
    exit(-1);
  }

  // Открываем семафор сервера, если не существует, создаём
  sem_t *sem_server = sem_open(SEM_SERVER, SEM_SERVER_FLAGS, SEM_FILE_MODE, 0);
  if (sem_server == SEM_FAILED) {
    ERROR("Cannot open server's semaphore: %s.", strerror(errno));
    exit(-1);
  }

  // Открываем или создаём разделяемую память
  int fd = shm_open(SHM, SHM_FLAGS, SHM_FILE_MODE);
  if (fd < 0) {
    ERROR("Cannot open shared memory: %s.", strerror(errno));
    exit(-1);
  }

  // Обозначаем длину разделяемой памяти
  ftruncate(fd, sizeof(Message));

  // Отображаем память в адресное пространство процесса
  Message *shm_ptr =
      mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_ptr == MAP_FAILED) {
    ERROR("Cannot map shared memory: %s.", strerror(errno));
    exit(-1);
  }

  Message msg;

  sprintf(msg.content, "%s", argv[2]);

  // Получаем реальный символ
  msg.symbol = argv[1][0];


  // Очищаем память
  memset(shm_ptr, 0, sizeof(Message));
  // Отправляем сообщение серверу в разделяему память
  *shm_ptr = msg;

  // Говорим серверу, что можно забирать сообщение
  if (sem_post(sem_client) < 0) {
    ERROR("Cannot touch client's semaphore: %s.", strerror(errno));
    exit_cleanup(-1, sem_server, sem_client, fd);
  }

  INFO("Message sent successfully.");

  // Если необходимо завершить работу
  if (!strcmp(msg.content, "shutdown"))
    exit_cleanup(-1, sem_server, sem_client, fd);

  // Говорим, что ждём сервер
  INFO("Wait for the server...");

  // Ожидаем ответа от сервера
  if (sem_wait(sem_server) < 0) {
    ERROR("Cannot wait for the server's semaphore: %s.",
           strerror(errno));
    exit_cleanup(-1, sem_server, sem_client, fd);
  }

  // Выводим на экран ответ сервера
  INFO("Answer from the server: \n%s.\n", shm_ptr->content);
}
