#include "../helpers/logging.h"

#include "cave.c"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

typedef struct {
  int capcity;
  Cave *cave;
} Worker;

Worker worker_init(int capacity, Cave *cave) {
  Worker worker;
  worker.capcity = capacity;
  worker.cave = cave;

  return worker;
}

void worker_thread(void *arg) {
  Worker bee = *(Worker*)arg;

  while (cave_modify_honey(bee.cave, bee.capcity) > 0) {

    // printf("[BEE]: Bee came back to hive.\n");

    sleep(rand() % 10 + 1);
  }

  printf("Worker finished.\n");
}
