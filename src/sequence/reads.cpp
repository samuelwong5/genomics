/* reads.cpp ----- James Arram 2015 */

#include "reads.hpp"

// set value in packed read
inline void setVal(uint8_t *pck, uint32_t idx, uint8_t val);

// test if captured new entry or a quality score
uint64_t testEntry(FILE *fp, char *buffer, uint64_t len); 

void loadReads(FILE *in_fp, std::vector<read_t> &reads, char *buffer, uint64_t size, 
	       bool ctrl, uint64_t *bytes, uint8_t *test)
{
  uint64_t len = size;
  uint64_t i;
  int c;
  
  reads.clear();
  
  if (ctrl == true) {
    
    // read file
    if (fread(&buffer[0], len, 1, in_fp) != 1) {
      printf("error: unable to read file!\n");
      exit(1);
    }
    
    // read until next record starts or eof
    while ((c = fgetc(in_fp)) != EOF) {
      if (c == '@') {
	ungetc(c, in_fp);
	len = testEntry(in_fp, buffer, len);
	break;
      }
      buffer[len++] = (char)c;
    }
    
    
    // parse buffer
    uint64_t s_pos;
    int s_len;
    read_t tmp;

    i = 0;
    while (i < len) {
        
      // get meta data line
      s_pos = i;
      s_len = 0;
      while (buffer[i++] != '\n') 
	s_len += 1;
      s_len = s_len > MAX_META ? MAX_META : s_len;
      memcpy(tmp.meta_data, &buffer[s_pos], s_len*sizeof(char));
      tmp.meta_data[s_len] = '\0';
      tmp.meta_len = s_len;
      
      // get sequence
      tmp.unknown = false;
      s_pos = i;
      s_len = 0;
      while (buffer[i] != '\n') {
	if (buffer[i++] == 'N')
	  tmp.unknown = true;
	s_len += 1;
      }
      i+=1;
      s_len = s_len > MAX_SEQ ? MAX_SEQ : s_len;            
      memcpy(tmp.sym, &buffer[s_pos], s_len*sizeof(char));
      tmp.sym[s_len] = '\0';
      tmp.seq_len = s_len;
      
      // get strand
      tmp.strand = buffer[i];
      if (buffer[i] == '+')
          *test |= 0x1;
      if (buffer[++i] == '\n')
          *test |= 0x2;
      while (buffer[i++] != '\n');      
      // get q_score
      s_pos = i;
      s_len = 0;
      while (buffer[i++] != '\n') {
	s_len += 1;
      }
      s_len = s_len > MAX_SEQ ? MAX_SEQ : s_len;
      memcpy(tmp.q_score, &buffer[s_pos], s_len*sizeof(char));
      tmp.q_score[s_len] = '\0';
      
      reads.push_back(tmp);
    }
    *bytes += len;
  }
}

// set value in packed read
inline void setVal(uint8_t *pck, uint32_t idx, uint8_t val)
{
  uint8_t tmp = val << ((idx*2)%8);
  pck[idx/4] |= tmp;
}

// pack read                                    
void pack(char *sym, uint8_t *pck, uint8_t len)
{
  memset(pck, 0, CEIL(len, 4)*sizeof(uint8_t));      
  
  for (uint32_t i = 0; i < len; i++) {
    switch(sym[len-1-i]) {
    case 'A': setVal(pck, i, 0); break;
    case 'C': setVal(pck, i, 1); break;
    case 'G': setVal(pck, i, 2); break;
    case 'T': setVal(pck, i, 3); break;
    default : setVal(pck, i, 0); 
    }
  }
}

// pack reverse complement read
void packRevComp(char *sym, uint8_t *pck, uint8_t len)
{
  memset(pck, 0, CEIL(len, 4)*sizeof(uint8_t));      
  
  for (uint32_t i = 0; i < len; i++) {
    switch (sym[i]) {
    case 'A': setVal(pck, i, 3); break;
    case 'C': setVal(pck, i, 2); break;
    case 'G': setVal(pck, i, 1); break;
    case 'T': setVal(pck, i, 0); break;
    default : setVal(pck, i, 0);
    }
  }
}

// reverse complement string
void revComp(char *sym, uint8_t len)
{
  char *p1 = sym;
  char *p2 = p1+len-1;
  char tmp1;
  char tmp2;
 
 while (p2 > p1) {
    tmp1 = *p1;
    tmp2 = *p2;
    
    switch (tmp1) {
    case 'A': *p2 = 'T'; break;
    case 'C': *p2 = 'G'; break;
    case 'G': *p2 = 'C'; break;
    case 'T': *p2 = 'A'; break;
    }
    
    switch (tmp2) {
    case 'A': *p1 = 'T'; break;
    case 'C': *p1 = 'G'; break;
    case 'G': *p1 = 'C'; break;
    case 'T': *p1 = 'A'; break;
    }
    
    p2--;
    p1++;
  }
}

// reverse string
void reverse(char *sym, uint8_t len)
{
  char *p1 = sym;
  char *p2 = p1+len-1;
  char tmp;
  
  while (p2 > p1) {
    tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p2--;
    p1++;
  }
}
 
// test if captured new entry or a quality score
uint64_t testEntry(FILE *fp, char *buffer, uint64_t len) 
{
  long int f_pos = ftell(fp);
  int cnt = 0;
  int c;
  
  // test if entry
  while ((c = fgetc(fp)) != EOF) {
    cnt += 1;
    if (c == '\n') {
      c = fgetc(fp);
      if (c != '@' && c != EOF)
	cnt = 0;
      break;
    }
  }

  // reset file pointer
  fseek(fp, f_pos, SEEK_SET);

  // read remaining positions
  for (int i = 0; i < cnt; i++)
    buffer[len++] = fgetc(fp);
  
  return len;
}
