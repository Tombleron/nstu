#include "../helpers/logging.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "info.h"


int main(int argc, char **argv) {

  if (argc < 3) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <character> [filenames]\n");
    exit(-1);
  }

  DBG("Creating FIFO channels");

  if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST)) {
    ERROR("Can't create %s", FIFO1);
    exit(-2);
  }

  if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
    unlink(FIFO1);
    ERROR("Can't create %s", FIFO2);
    exit(-2);
  }

  DBG("Openning channels.")

  int tx = open(FIFO2, O_WRONLY);
  int rx = open(FIFO1, O_RDONLY);

  char buf[BUF_LENGTH];

  int total = 0;

  for (int i = 2; i < argc; i++) {
    buf[0] = argv[1][0];
    sprintf(buf+1, "%s", argv[i]);
    int n = strlen(buf);
    DBG("Sending message with contents: <%s>", buf);
    write(tx, buf, n);

    DBG("Reading message from channel.");

    read(rx, buf, BUF_LENGTH);
    DBG("Got message: <%s>.", buf);

    int got = atoi(buf);

    if (got < 0) {
      ERROR("Child got error while processing file. Error code: %d", got);
    } else {
      total += got;
    }

  }

  
  INFO("Total is: %d", total);
  return total;
}