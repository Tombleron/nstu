#include "../helpers/logging.h"

#include "hive.c"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

typedef struct {
  int capcity;
  Hive *hive;
} Bee;

Bee bee_init(int capacity, Hive *hive) {
  Bee bee;
  bee.capcity = capacity;
  bee.hive = hive;

  return bee;
}

void bee_thread(void *arg) {
  Bee bee = *(Bee *)arg;

  while (hive_modify_honey(bee.hive, bee.capcity) > 0) {

    // printf("[BEE]: Bee came back to hive.\n");

    sleep(rand() % 5 + 5);
  }

  printf("[BEE]: !");
}
