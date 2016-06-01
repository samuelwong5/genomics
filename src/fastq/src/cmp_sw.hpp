/* cmp_sw.hpp ----- James Arram 2016 */

#ifndef CMP_SW_HPP
#define CMP_SW_HPP

#include "def.hpp"
#include "inc.hpp"
#include "reads.hpp"
#include "index.hpp"

// compress reads
void compress(std::vector<read_t> &reads, index_t *index, ival_t *ival1, ival_t *ival2,
	      uint32_t *sai, char *f_prefix);
  
#endif