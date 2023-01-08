#ifndef HIVE
#define HIVE

#include "../helpers/logging.h"
#include "pthread.h"
#include <stdio.h>

typedef struct {
  int gold;
  pthread_mutex_t mutex;
} Cave;

Cave cave_init(int reserves) {
  Cave cave;
  cave.gold = reserves;
  pthread_mutex_init(&cave.mutex, NULL);

  return cave;
}

void cave_lock(Cave *hive) {
  pthread_mutex_lock(&hive->mutex);
  DBG("Cave locked.");
}

void cave_unlock(Cave *hive) {
  pthread_mutex_unlock(&hive->mutex);
  DBG("Cave unlocked.");
}

int cave_modify_honey(Cave *hive, int count) {
  cave_lock(hive);

  int gold;
  if (hive->gold >= count) {
    hive->gold -= count;
    gold = hive->gold;
    printf("[CAVE]: gold count: %d\n", gold);
  } else {
    gold = -1;
    printf("[CAVE]: Not enough gold\n");
  } 

  cave_unlock(hive);

  return gold;
}

#endif /* !HIVE */
