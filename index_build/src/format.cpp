/* faToFmt.cpp ----- James Arram 2016 */

#include "format.hpp"

#define LINE_SIZE 256
#define MAX_UN 10
#define UN_CH 'G'

// format fasta file
void format(char *f_prefix, char *ref, uint64_t *len)
{
  FILE *fp = NULL;
  char line[LINE_SIZE];
  uint32_t un_cnt = 0;
  uint64_t cnt = 0;

  // open fasta file
  fp = fopen(f_prefix, "r");
  if (!fp) {
    printf("error: unable to open file '%s'!\n", f_prefix);
    exit(1);
  }
  
  // process lines
  printf("formatting reference sequence ... "); fflush(stdout);
  while (fgets(line, LINE_SIZE, fp) != NULL) {
    if (line[0] != '>') {
      uint32_t i = 0;
      while (line[i] > 32) {
        line[i] = (char)((line[i] & 0x1F) | 0x40);
        if (line[i] == 'N') {
          un_cnt++;
        }
        else {
          if (un_cnt <= MAX_UN) {
            while (un_cnt > 0) {
	      ref[cnt++] = UN_CH;
              un_cnt-=1;
            }
          }
	  else {
            un_cnt = 0;
	  }
	  switch (line[i]) {
	  case 'A': ref[cnt++] = 'A'; break;
	  case 'C': ref[cnt++] = 'C'; break;
	  case 'G': ref[cnt++] = 'G'; break;
	  case 'T': ref[cnt++] = 'T'; break;
	  default: ref[cnt++] = UN_CH;
	  }	  
	}
	i+=1;
      }
    }
  }

  if (feof(fp) && !ferror(fp)) {
    if (un_cnt <= MAX_UN) {
      while (un_cnt > 0) {
	ref[cnt++] = UN_CH;
	un_cnt-=1;
      }
    }
  }
  fclose(fp);

  // write reference to file
  char f_name[128];
  sprintf(f_name, "%s%s", f_prefix, ".ref");
  fp = fopen(f_name, "wb");
  if (!fp) {
    printf("error: unable to open file '%s'!\n", f_name);
    exit(1);
  }
  if (fwrite(ref, cnt*sizeof(char), 1, fp) != 1) {
    printf("error: unable to write to '%s'!\n", f_name);
    exit(1);
  } 
  fclose(fp);

  ref[cnt++] = '$';
  *len = cnt;

  printf("done\n");
}