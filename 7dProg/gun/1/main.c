#include "fcntl.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#ifdef DEBUG
#define DBG(a, ...)                                                            \
  { printf(a "\n", ##__VA_ARGS__); }
#else
#define DBG(a, ...)                                                            \
  {}
#endif

#define INFO(a, ...)                                                           \
  { printf("[INFO]: " a "\n", __VA_ARGS__); }

#define ERROR(a, ...)                                                          \
  { printf("[ERROR]: " a "\n", ##__VA_ARGS__); }

#define BUF_SIZE 512

const char PUNCT_CHARS[] = {'!', '?', '.', ',', ';', ':', '-', 0};

_Bool is_punctuatuion(char ch) {
  for (int i = 0; PUNCT_CHARS[i] != 0; i++) {
    if (ch == PUNCT_CHARS[i]) {
      return true;
    }
  }
  return false;
}

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

  ssize_t bytes_red;
  char buffer[BUF_SIZE];
  int count = 0;

  DBG("Trying to process files.");

  // Read chunk of file
  while ((bytes_red = read(input_desc, buffer, BUF_SIZE)) > 0) {

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
    if (write(output_desc, buffer, bytes_red) != bytes_red) {
      ERROR("Failed to write chunk to output file.");
      return -4;
    }
  }

  if (bytes_red == -1) {
    ERROR("Failed to read input file.");
    return -5;
  }

  DBG("Closing descriptors.");

  // Close descriptors
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