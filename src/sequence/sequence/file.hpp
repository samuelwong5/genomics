/* file.hpp ----- James Arram 2016 */

/*
 * file utilities
 */

#ifndef FILE_HPP
#define FILE_HPP

#include "inc.hpp"

// open file 
void openFile(FILE **fp, char *f_name, const char *mode);

// get file size in bytes
uint64_t fileSizeBytes(FILE *fp);

// read file
void readFile(FILE *fp, void  *a, uint64_t size);

// write file
void writeFile(FILE *fp, void *a, uint64_t size);

// map suffix array to memory
void mapSuffixArray(char *f_name, uint32_t **sai, uint64_t *map_size);

// map reference genome to memory
void mapRef(char *f_name, char **ref, uint64_t *map_size);

#endif