#include "../helpers/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {

  if (argc < 3) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <character> [filenames]\n");
    exit(-1);
  }

  DBG("Allocating memory for pids array.")
  // Allocate array for pids
  int *pids = malloc((argc - 2) * sizeof(int));

  // Iterate over all filenames
  for (int i = 0; i < argc - 2; i++) {
    DBG("Starting new child process.");
    if ((pids[i] = fork()) == 0) {
      DBG(" <%d> Running executable with <%s %s> arguments.", getpid(),
          argv[i + 2], argv[1]);
      INFO("Starting new child.");
      if (execl("./child.out", "child.out", argv[i + 2], argv[1], NULL) < 0) {
        ERROR(" <%d> Error starting executable.", getpid());
        exit(-10);
      }
    }
  }

  DBG("<%d>Waiting fo all children to die.", getpid());
  for (int i = 0; i < argc - 2; i++) {

    DBG("Waiting for %d.", pids[i]);

    int status;
    int pid = waitpid(pids[i], &status, 0);

    if (pids[i] == pid) {
      INFO("File %s done,  result=%d\n", argv[i+2], WEXITSTATUS(status));
    }

  }
}