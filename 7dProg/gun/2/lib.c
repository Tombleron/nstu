#include <stdio.h>
#include <unistd.h>
#include "stdbool.h"

#include "../helpers/logging.h"

const char PUNCT_CHARS[] = {'!', '?', '.', ',', ';', ':', '-', 0};

_Bool is_punctuatuion(char ch) {
  for (int i = 0; PUNCT_CHARS[i] != 0; i++) {
    if (ch == PUNCT_CHARS[i]) {
      return true;
    }
  }
  return false;
}


int process_file(int in_desc, int out_desc, char sub_ch, char *buffer, int buffer_size) {

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