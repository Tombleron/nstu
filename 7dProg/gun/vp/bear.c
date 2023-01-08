#include "../helpers/logging.h"

#include "hive.c"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

typedef struct {
  int appetite;
  int delay;
  // int saturation;
  Hive *hive;
} Bear;

Bear bear_init(int delay, int appetite, Hive* hive) {
  Bear bear;
  // bear.saturation = init_saturation;
  bear.appetite = appetite;
  bear.delay = delay;
  bear.hive = hive;

  return bear;
}

void bear_thread(void *arg) {
  Bear bear = *(Bear *)arg;

  while (hive_modify_honey(bear.hive, -bear.appetite) >= 0) {

    printf("[BEAR]: Bear ate %d units of honey.\n", bear.appetite);
    // printf("[BEAR]: Bear saturation is %d units of honey.", bear.saturation);
    // bear.saturation += bear.appetite;

    sleep(bear.delay);
  }

  printf("[BEAR]: The bear is dead, long live the bear!\n");
}
