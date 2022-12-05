#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stdbool.h"

#include "../helpers/logging.h"

int open_files(char* input_filename, int* input_desc, int* output_desc) {
  // Input and output files descriptors

  if ((*input_desc = open(input_filename, O_RDONLY)) == -1) {
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

  if ((*output_desc = open(output_filename, flags, mode)) == -1) {
    ERROR("Failed to open output file.");
    return -3;
  }

  return 0;
}

const char PUNCT_CHARS[] = {'!', '?', '.', ',', ';', ':', '-', 0};

_Bool is_punctuatuion(char ch) {
  for (int i = 0; PUNCT_CHARS[i] != 0; i++) {
    if (ch == PUNCT_CHARS[i]) {
      return true;
    }
  }
  return false;
}


int process_file(int in_desc, int out_desc, char sub_ch) {
  int buffer_size = 512;
  char buffer[buffer_size];
  ssize_t bytes_red, count = 0;
  while ((bytes_red = read(in_desc, buffer, buffer_size)) > 0) {

    DBG("Red %ld bytes from input file.", bytes_red);

    // Process chunk
    for (int i = 0; i < bytes_red; i++) {
      if (is_punctuatuion(buffer[i])) {
        buffer[i] = sub_ch;
        count++;
      }
    }

    DBG("Writing buffer to output file.");

    // Write buffer to file
    if (write(out_desc, buffer, bytes_red) != bytes_red) {
      ERROR("Failed to write chunk to output file.");
      return -4;
    }

  }

  if (bytes_red == -1) {
    ERROR("Failed to read input file.");
    return -5;
  }

  return count;
}