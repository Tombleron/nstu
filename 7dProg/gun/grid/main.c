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

#include "worker.c"

int main(int argc, char **argv) {

  DBG("argc = %d", argc);

  for (int i = 0; i < argc; i++) {
    DBG("argv[%d] = %s", i, argv[i]);
  }

  if (argc < 4) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <worker capacity> <workers count> <cave reserves>"
           "capacity>\n");
    exit(-1);
  }

  int capacity = atoi(argv[1]);
  int count = atoi(argv[2]);
  int reserves = atoi(argv[3]);

  Worker *workers = calloc(count, sizeof(Worker));

  pthread_t *threads = calloc(count, sizeof(pthread_t));

  Cave cave = cave_init(reserves);

  for (int i = 0; i < count; i++) {

    DBG("Starting worker %d", i);

    workers[i] = worker_init(capacity, &cave);

    // Запускаем поток
    if (pthread_create(&threads[i], NULL, (void *)worker_thread, &workers[i]) != 0) {
      ERROR("Cannot create worker #%d.", i);
      exit(-1);
    }
  }

  DBG("Waiting for results");

  void *result;
  // Запускаем цикл ожидания работы всех потоков
  for (int i = 0; i < count; i++) {
    // Ожидаем завершения потока
    DBG("Waiting for thread <%d>", i);
    pthread_join(threads[i], &result);
  }
}
