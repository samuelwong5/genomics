/* index.hpp ----- James Arram 2016 */

/*
 * Notes:
 * number of steps for FM-index is 3
 */

#ifndef INDEX_H
#define INDEX_H

#include "inc.hpp"
#include "def.hpp"

// suffix array interval
struct ival_t {
  uint32_t low;
  uint32_t high;
};

// index structure
struct index_t {
  uint32_t counters[64];
  uint8_t bwt[CEIL(BUCKET_SIZE*7,8)];
  uint32_t pad;
};

#endif