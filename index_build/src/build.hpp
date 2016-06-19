/* faToFmt.hpp ----- James Arram 2016 */

/*
 * Function:
 * Generate 3-step FM-index of reference sequence
 *
 * Usage: 
 * ./fmtToIdx <fmt file>
 *
 * Notes:
 * 1) Bucket size is 288 characters
 * 2) bwt symbols are encoded with 7 bits
 */

#ifndef BUILD_HPP
#define BUILD_HPP
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <omp.h>
#include "divsufsort64.h"

// build index
void build(char *f_name, char *ref, uint64_t len);

#endif

