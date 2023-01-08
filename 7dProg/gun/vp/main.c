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

#include "bear.c"
#include "bee.c"

int main(int argc, char **argv) {

  DBG("argc = %d", argc);

  for (int i = 0; i < argc; i++) {
    DBG("argv[%d] = %s", i, argv[i]);
  }

  if (argc < 4) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <bear appetite> <bee count> <bee "
           "capacity>\n");
    exit(-1);
  }

  int appetite = atoi(argv[1]);
  int count = atoi(argv[2]);
  int capacity = atoi(argv[3]);

  Bee *bees = calloc(count, sizeof(Bee));

  pthread_t *threads = calloc(count + 1, sizeof(pthread_t));

  Hive hive = hive_init();

  for (int i = 0; i < count; i++) {

    DBG("Starting bee %d", i);

    bees[i] = bee_init(capacity, &hive);

    // Запускаем поток
    if (pthread_create(&threads[i], NULL, (void *)bee_thread, &bees[i]) != 0) {
      ERROR("Cannot create bee #%d.", i);
      exit(-1);
    }
  }

  sleep(5);

  Bear bear = bear_init(5, appetite, &hive);

  if (pthread_create(&threads[count], NULL, (void *)bear_thread, &bear) != 0) {
    ERROR("Cannot create bear.");
    exit(-1);
  }

  DBG("Waiting for results");

  void *result;
  // Запускаем цикл ожидания работы всех потоков
  for (int i = 0; i < count + 1; i++) {
    // Ожидаем завершения потока
    DBG("Waiting for thread <%d>", i);
    pthread_join(threads[i], &result);
  }
}
