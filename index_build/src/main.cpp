/* main.cpp ----- James Arram 2016 */

#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "format.hpp"
#include "build.hpp"

int main(int argc, char *argv[]) {

  char *ref = NULL;

  // program usage
  if (argc != 2) {
    printf("usage: %s <fasta file>\n", argv[0]);
    exit(1);
  }
  
  // allocate memory for reference sequence
  struct stat st;
  uint64_t len;
  if (stat(argv[1], &st) != 0) {
    printf("error: unable to get '%s' file size!\n", argv[1]);
    exit(1);
  }
  len = st.st_size;
  ref = new char [len*sizeof(char)];
  if (!ref) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }
  
  // format reference sequence
  uint64_t ref_len = 0;
  format(argv[1], ref, &ref_len);

  // build index
  build(argv[1], ref, ref_len);

  // cleanup
  delete[] ref;

  return 0;
}

