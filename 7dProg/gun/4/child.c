#include "../helpers/logging.h"
#include "info.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lib.c"

int main(int argc, char **argv) {

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

  int rx = open(FIFO2, O_RDONLY);
  int tx = open(FIFO1, O_WRONLY);

  int n = 0;
  char buf[BUF_LENGTH];

  while ((n = read(rx, buf, BUF_LENGTH)) > 0) {
    DBG("Got message with content: <%s>", buf);
    
    int in_desc, out_desc;
    if(open_files(buf+1, &in_desc, &out_desc) < 0) {
      ERROR("Error opening files.");
      exit(-3);
    };
    if ((n = process_file(in_desc, out_desc, buf[0])) < 0) {
      ERROR("Error processing files.");
    }
    sprintf(buf, "%d", n);
    write(tx, buf, strlen(buf)+1);

  }
}