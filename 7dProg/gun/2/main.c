#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#ifdef DYNAMIC
#include "dlfcn.h"
#endif
// Logging macros
#include "../helpers/logging.h"

#define BUF_SIZE 512

#ifndef DYNAMIC
int process_file(int in_desc, int out_desc, char sub_ch, char *buffer,
                 int buffer_size);
#endif

int main(int argc, char **argv) {

  if (argc < 3) {
    printf("Not enough arguments.\n"
           "\t\t\tUSAGE: ./executable <filename> <character>\n");
    return -1;
  }

  char *input_filename = argv[1];
  char sub_ch = argv[2][0];

  DBG("Input file: %s\nCharacter: %c", input_filename, sub_ch);

  DBG("Trying to open input file");

  // Input and output files descriptors
  int input_desc, output_desc;

  if ((input_desc = open(input_filename, O_RDONLY)) == -1) {
    ERROR("Failed to open input file.");
    return -2;
  }

  // Flags:
  //  Create if doesn't exist
  //  Open only for write
  //  If exists truncate to zero size
  int flags = O_CREAT | O_WRONLY | O_TRUNC;

  // Mode:
  //  User has write permissions
  //  User has read permissions
  //  Group has write permissions
  //  Group has read permissions
  //  Others has write permissions
  //  Others has read permissions
  mode_t mode = S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH;

  DBG("Trying to open output file.");

  // Create output filename
  char *output_filename = malloc(strlen(input_filename) + 4);
  strcpy(output_filename, input_filename);
  strcat(output_filename, ".out");

  DBG("Output filename: %s", output_filename);

  if ((output_desc = open(output_filename, flags, mode)) == -1) {
    ERROR("Failed to open output file.");
    return -3;
  }

  DBG("Trying to process files.");

  char buffer[BUF_SIZE];

#ifdef DYNAMIC
  DBG("Trying to open dynamic library.")
  void *ext_lib = dlopen("./lib.so", RTLD_LAZY);
  if (!ext_lib) {
    ERROR("Failed to load shared library.")
    return -8;
  }
  DBG("Trying to load function from dynamic library.")
  int (*process_file)(int, int, char, char *, int) =
      dlsym(ext_lib, "process_file");
#endif

  int count = process_file(input_desc, output_desc, sub_ch, buffer, BUF_SIZE);

  DBG("Closing descriptors.");

#ifdef DYNAMIC
  dlclose(ext_lib);
#endif

  if (close(input_desc) == -1) {
    ERROR("Failed to close input descriptor.");
    exit(-6);
  }

  if (close(output_desc) == -1) {
    ERROR("Failed to close output descriptor.");
    exit(-7);
  }

  return count;
}