#ifndef HIVE
#define HIVE



#include "../helpers/logging.h"
#include "pthread.h"
#include <stdio.h>

typedef struct {
  int honey;
  pthread_mutex_t mutex;
} Hive;

Hive hive_init() {
  Hive hive;
  hive.honey = 0;
  pthread_mutex_init(&hive.mutex, NULL);

  return hive;
}

void hive_lock(Hive *hive) {
  pthread_mutex_lock(&hive->mutex);
  DBG("Hive locked.");
}

void hive_unlock(Hive *hive) {
  pthread_mutex_unlock(&hive->mutex);
  DBG("Hive unlocked.");
}

int hive_read_honey(Hive *hive) {
  hive_lock(hive);

  int honey = hive->honey;
  DBG("Hive count: %d", honey);

  hive_unlock(hive);

  return honey;
}

int hive_modify_honey(Hive *hive, int count) {
  hive_lock(hive);

  int honey;
  if (hive->honey >= -count) {
    hive->honey += count;
    honey = hive->honey;
    printf("[HIVE]: honey count: %d\n", honey);
  } else {
    honey = -1;
    printf("[HIVE]: Not enough honey\n");
  } 

  hive_unlock(hive);

  return honey;
}

#endif /* !HIVE */
