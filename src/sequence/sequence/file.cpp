/* file.cpp ----- James Arram 2015 */

#include "file.hpp"

// open file 
void openFile(FILE **fp, char *f_name, const char *mode)
{
  *fp = fopen(f_name, mode);
  if (!(*fp)) {
    printf("error: unable to open file '%s'!\n", f_name);
    exit(1);
  }
  //printf("FP: %p\n", *fp);
  setvbuf(*fp, NULL, _IOFBF, 1024*1024*64);
}

// get file size in bytes
uint64_t fileSizeBytes(FILE *fp)
{
  struct stat st;
  uint64_t len;
  int fd;

  if ((fd = fileno(fp)) == -1) {
    printf("error: unable to get file size!\n");
    exit(1);
  }

  if(fstat(fd, &st)) { 
    printf("error: unable to get file size!\n"); 
    exit(1);
  } 
  
  len = st.st_size;

  return len;
}

// read file
void readFile(FILE *fp, void *a, uint64_t n_bytes)
{
  //printf("readfile fp: %p\n", fp);
  if (fread(a, n_bytes, 1, fp) != 1) {
    printf("error: unable to read file!\n");
    exit(1);
  }
}

// write file
void writeFile(FILE *fp, void *a, uint64_t n_bytes)
{
  if (fwrite(a, n_bytes, 1, fp) != 1) {
    printf("error: unable to write file!\n");
    exit(1);
  }
}

// map suffix array to memory
void mapSuffixArray(char *f_name, uint32_t **sai, uint64_t *map_size)
{
  uint64_t f_size;
  int fd, page_size;
  struct stat st;

  fd = open(f_name, O_RDONLY);
  if (fd == -1) {
    printf("error: unable to open file\n");
    exit(1);
  }
  if(fstat(fd, &st)) { 
    printf("error: unable to get file size!\n"); 
    exit(1);
  }   
  f_size = st.st_size;
  page_size = getpagesize();
  f_size += page_size-(f_size%page_size);
  *map_size = f_size;

  *sai = (uint32_t*) mmap(0, f_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (*sai == MAP_FAILED) {
    close(fd);
    printf("error: unable to map file\n");
    exit(1);
  }
  close(fd);
}

// map reference genome to memory
void mapRef(char *f_name, char **ref, uint64_t *map_size)
{
  uint64_t f_size;
  int fd, page_size;
  struct stat st;

  fd = open(f_name, O_RDONLY);
  if (fd == -1) {
    printf("error: unable to open file\n");
    exit(1);
  }
  if(fstat(fd, &st)) { 
    printf("error: unable to get file size!\n"); 
    exit(1);
  }   
  f_size = st.st_size;
  page_size = getpagesize();
  f_size += page_size-(f_size%page_size);
  *map_size = f_size;

  *ref = (char*) mmap(0, f_size, PROT_READ, MAP_PRIVATE|MAP_LOCKED, fd, 0);
  if (*ref == MAP_FAILED) {
    close(fd);
    printf("error: unable to map file\n");
    exit(1);
  }
  close(fd);
}



 
