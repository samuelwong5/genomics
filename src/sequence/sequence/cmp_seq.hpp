/* cmp_seq.hpp ----- James Arram 2016 */

#ifndef CMP_SEQ_HPP
#define CMP_SEQ_HPP

#include "def.hpp"
#include "inc.hpp"
#include "reads.hpp"
#include "index.hpp"

struct dcmp_fps {
    FILE *fp[5];
    char *ref;
};

void decompress_init(dcmp_fps&, char*, char*);

// compress reads
void compressSeq(std::vector<read_t> &reads, index_t *index, ival_t *ival1, ival_t *ival2,
		 uint32_t *sai, char *f_prefix);

// decompress reads
void decompressSeq(std::vector<read_t> &reads, char *ref, dcmp_fps&);
  
#endif
