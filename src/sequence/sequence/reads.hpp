/* reads.hpp ----- James Arram 2014 */

#ifndef READS_H
#define READS_H

#include "def.hpp"
#include "inc.hpp"
#include "file.hpp"

// tuple
struct tuple_t {
  uint32_t pos;
  uint8_t len;
  uint8_t sym;
};

// read
struct read_t {
  char meta_data[MAX_META + 1];
  char sym[MAX_SEQ + 1];
  char strand;
  char q_score[MAX_SEQ + 1];
  uint8_t seq_len;
  uint8_t meta_len;
  bool unknown; 
};

// load reads
void loadReads(FILE *in_fp, std::vector<read_t> &reads, char *buffer, uint64_t size, 
	       bool ctrl, uint64_t *bytes, uint8_t *test);

// pack read                                    
void pack(char *sym, uint8_t *pck, uint8_t len);

// pack reverse complement read
void packRevComp(char *sym, uint8_t *pck, uint8_t len);

// reverse complement string
void revComp(char *sym, uint8_t len);

// reverse string
void reverse(char *sym, uint8_t len);

#endif
