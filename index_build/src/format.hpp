/* faToFmt.hpp ----- James Arram 2016 */

/*
 * Function:
 * Format FASTA file 

 * Notes:
 * 1) It is assumed that the fasta file is correctly formatted
 * 2) Any unknown characters are converted to 'G'
 * 3) Any continuous sequence of unknown characters > 10 is deleted
 * 4) All line breaks and header lines are removed
 * 6) Input file must have fewer than 2^32 characters
 */


#ifndef FORMAT_HPP
#define FORMAT_HPP
 
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include <string>
#include <stdint.h>

// format fasta file
void format(char *f_name, char *ref, uint64_t *len);

#endif