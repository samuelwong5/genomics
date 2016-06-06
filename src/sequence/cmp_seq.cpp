/* cmp_seq.cpp ----- James Arram 2016 */

#include "cmp_seq.hpp"

// align sequence
void alignSW(char *sym, uint8_t start, uint8_t len, index_t *index, ival_t *ival1, 
	     ival_t *ival2, std::vector<tuple_t> &tuples);

// compress sequence with unknown symbols
void compressUnknown(char *seq, uint8_t len, index_t *index, ival_t *ival1,
		     ival_t *ival2, std::vector<tuple_t> &tuples);

// count occurrence from BWT
uint32_t getOcc(uint8_t sym, uint8_t *bwt, uint32_t s_idx, uint32_t e_idx);

// get value in packed BWT
inline uint8_t getVal(uint8_t *bwt, uint32_t idx);


// compress reads
void compressSeq(std::vector<read_t> &reads, index_t *index, ival_t *ival1, ival_t *ival2,
		 uint32_t *sai, char *f_prefix)
{
  FILE *fp[5];
  std::vector<std::vector<tuple_t> > tuples(reads.size());
  std::vector<uint8_t> orn(reads.size(), 0);

  // compress reads
#pragma omp parallel for num_threads(N_THREADS)
  for (uint32_t i = 0; i < reads.size(); i++) {
    if (reads[i].unknown == false)
      alignSW(reads[i].sym, 0, reads[i].seq_len, index, ival1, ival2, tuples[i]);
    else 
      compressUnknown(reads[i].sym, reads[i].seq_len, index, ival1, ival2, tuples[i]); 
  }

  // compress reverse complement reads
#pragma omp parallel for num_threads(N_THREADS)
  for (uint32_t i = 0; i < reads.size(); i++) {
    if (tuples[i].size() > 1 && reads[i].unknown == false) {
      std::vector<tuple_t> tmp;
      char sym_rc[MAX_SEQ+1];
      strcpy(sym_rc, reads[i].sym);
      revComp(sym_rc, reads[i].seq_len);
      alignSW(sym_rc, 0, reads[i].seq_len, index, ival1, ival2, tmp);
      if (tmp.size() < tuples[i].size()) {
	tuples[i] = tmp;
	orn[i] = 1;
      }
    }
  }

  // convert suffix array index to reference position
#pragma omp parallel for num_threads(N_THREADS)
  for (uint32_t i = 0; i < reads.size(); i++) {
    for (uint32_t j = 0; j < tuples[i].size(); j++) {
      tuples[i][j].pos = sai[tuples[i][j].pos];
    }
  }

  // write compression results to file
  uint32_t tuple_cnt = 0;
#pragma omp parallel for reduction(+:tuple_cnt) num_threads(N_THREADS)
  for (uint32_t i = 0; i < reads.size(); i++) {
    tuple_cnt = tuple_cnt + tuples[i].size();
  }
  
  std::vector<uint32_t> pos(tuple_cnt);
  std::vector<uint8_t> len(tuple_cnt);
  std::vector<uint8_t> sym(tuple_cnt);
  std::vector<uint8_t> n_tuple(reads.size());

  // flatten tuples
  for (uint32_t i = 0, k = 0; i < reads.size(); i++) {
    n_tuple[i] = tuples[i].size();
    for (uint32_t j = 0; j < tuples[i].size(); j++) {
      pos[k] = tuples[i][j].pos;
      len[k] = tuples[i][j].len;
      sym[k] = tuples[i][j].sym;
      k++;
    }
  }
  for (int i = 0; i < 5; i++)
      printf("%u ", pos[i]);
  

  const char *suff[] = {".pos", ".len", ".sym", ".cnt", ".orn"};
  char f_name[128];
  for (int i = 0; i < 5; i++) {
    sprintf(f_name, "%s%s", f_prefix, suff[i]);
    openFile(&fp[i], f_name, "ab");
  }
  printf("Tuple count: %d\n", tuple_cnt);
  writeFile(fp[0], &pos[0], tuple_cnt*sizeof(uint32_t));
  writeFile(fp[1], &len[0], tuple_cnt*sizeof(uint8_t));
  writeFile(fp[2], &sym[0], tuple_cnt*sizeof(uint8_t));
  writeFile(fp[3], &n_tuple[0], reads.size()*sizeof(uint8_t));
  writeFile(fp[4], &orn[0], reads.size()*sizeof(uint8_t));
  
  // cleanup 
  for (int i = 0; i < 5; i++)
    fclose(fp[i]);
}

// compress sequence with unknown symbols
void compressUnknown(char *sym, uint8_t len, index_t *index, ival_t *ival1, ival_t *ival2,
		     std::vector<tuple_t> &tuples)
{
  tuple_t tmp;
  uint8_t n_cnt, cnt, i;
  
  cnt = 0;
  n_cnt = 0;
  i = 0;
  
  while (i < len) {   
  
    // unknown symbol
    if (sym[len-1-i] == 'N') {
      while(sym[len-1-i] == 'N' && i < len) {
	n_cnt += 1;
	i += 1;
      }
      tmp.pos = 0;
      tmp.len = n_cnt;
      tmp.sym = N_SYM;
      tuples.push_back(tmp);
    }
     
    // sequence symbol
    else {
      while(sym[len-1-i] != 'N' && i < len) {
	cnt += 1;
	i += 1;
      }
      alignSW(sym, len-i, cnt, index, ival1, ival2, tuples);
    }

    cnt = 0;
    n_cnt = 0;
  }
}

// align sequence
void alignSW(char *sym, uint8_t start, uint8_t len, index_t *index, ival_t *ival1, 
	     ival_t *ival2, std::vector<tuple_t> &tuples)
{
  // intialise suffix array interval
  uint8_t offset = len%3;

  uint8_t sym1_init = 0;
  uint8_t sym2_init = 0;

  if (len > 1) {
    switch (sym[start+len-2]) {
    case 'A' : sym2_init = 0; break;
    case 'C' : sym2_init = 4; break;
    case 'G' : sym2_init = 8; break;
    case 'T' : sym2_init = 12; break;
    default : return;
    }
  }

  switch (sym[start+len-1]) {
  case 'A' : sym2_init += 0; sym1_init = 0; break;
  case 'C' : sym2_init += 1; sym1_init = 1; break;
  case 'G' : sym2_init += 2; sym1_init = 2; break;
  case 'T' : sym2_init += 3; sym1_init = 3; break;
  default : return;
  }
 
  uint32_t low = offset == 0 ? 0 :
    offset == 1 ? ival1[sym1_init].low : ival2[sym2_init].low;
  uint32_t high = offset == 0 ? ival1[3].high : 
    offset == 1 ? ival1[sym1_init].high : ival2[sym2_init].high;
  uint8_t map_len = offset;

  if (offset == len) {
    tuple_t tmp;
    tmp.pos = high;
    tmp.len = len;
    tmp.sym = 64;
    tuples.push_back(tmp);
    return;
  }

  for (uint8_t i = offset; i < len; i+=3) {
    
    // get read symbol
    uint8_t sym_val;
    switch (sym[start+len-i-3]) {
    case 'A' : sym_val = 0; break;
    case 'C' : sym_val = 16; break;
    case 'G' : sym_val = 32; break;
    case 'T' : sym_val = 48; break;
    default : return;
    }
    switch (sym[start+len-i-2]) {
    case 'A' : sym_val += 0; break;
    case 'C' : sym_val += 4; break;
    case 'G' : sym_val += 8; break;
    case 'T' : sym_val += 12; break;
    default : return;
    }
    switch (sym[start+len-i-1]) {
    case 'A' : sym_val += 0; break;
    case 'C' : sym_val += 1; break;
    case 'G' : sym_val += 2; break;
    case 'T' : sym_val += 3; break;
    default : return;
    }

    uint32_t low_tmp = low == 0 ? 0 : low - 1;
    bool same_bucket = high - low_tmp < BUCKET_SIZE/OS_FACTOR ? true : false;
    
    // update low
    uint32_t low_addr = low_tmp/(BUCKET_SIZE/OS_FACTOR); 
    uint32_t low_idx = low_tmp - ((low_tmp/(BUCKET_SIZE/OS_FACTOR))*(BUCKET_SIZE/OS_FACTOR));
    uint32_t low_counter = index[low_addr].counters[sym_val];
    uint32_t low_count = getOcc(sym_val, index[low_addr].bwt, 0, low_idx);
    uint32_t low_new = low == 0 ? low_counter : low_counter + low_count;
    
    // update high
    uint32_t high_idx;
    uint32_t high_new;
    if (same_bucket) {
      high_idx = high - ((low_tmp/(BUCKET_SIZE/OS_FACTOR))*(BUCKET_SIZE/OS_FACTOR));
      high_new = low_counter + low_count + 
	getOcc(sym_val, index[low_addr].bwt, low_idx+1, high_idx) - 1;
    }   
    else {
      uint32_t high_addr = high/(BUCKET_SIZE/OS_FACTOR);
      high_idx = high - ((high/(BUCKET_SIZE/OS_FACTOR))*(BUCKET_SIZE/OS_FACTOR));
      high_new = index[high_addr].counters[sym_val] + 
	getOcc(sym_val, index[high_addr].bwt, 0, high_idx) - 1;
    }
 
    // update alignment state
    bool is_align = low_new <= high_new ? true : false;
    if (i == len-3) {
      tuple_t tmp;
      if (is_align == true) {
	tmp.pos = high_new;
	tmp.len = map_len + 3;
	tmp.sym = 64;
      }
      else {
	tmp.pos = high;
	tmp.len = map_len;
	tmp.sym = sym_val;
      }
      tuples.push_back(tmp);
      return;
    }
    
    if (is_align == true) {
      low = low_new;
      high = high_new;
      map_len += 3;
    }
    else {
      tuple_t tmp;
      tmp.pos = high;
      tmp.len = map_len;
      tmp.sym = sym_val;
      tuples.push_back(tmp);
      
      low = 0;
      high = ival1[3].high;
      map_len = 0;
    }
  }
}

// count occurrence from BWT
uint32_t getOcc(uint8_t sym, uint8_t *bwt, uint32_t s_idx, uint32_t e_idx)
{
  uint32_t cnt = 0;
  
  for (uint32_t i = s_idx; i <= e_idx; i++) {
    uint8_t bwt_sym = getVal(bwt, i);
    if (sym == bwt_sym) {
      cnt += 1;
    }
  }
  return cnt;
}

// get value in packed bwt
inline uint8_t getVal(uint8_t *bwt, uint32_t idx)
{
  uint16_t tmp = (((uint16_t)bwt[((idx*7)/8)+1]) << 8) | bwt[(idx*7)/8];
  tmp = (tmp >> ((idx*7)%8)) & 0x7f;
  return tmp;
}


void decompress_init(dcmp_fps& fps, char *f_prefix, char *fmt)
{   
    char f_name[128];
    sprintf(f_name, "%s%s", f_prefix, ".pos");
    openFile(&fps.fp[0], f_name, "rb");
    sprintf(f_name, "%s%s", f_prefix, ".len");
    openFile(&fps.fp[1], f_name, "rb");
    
    sprintf(f_name, "%s%s", f_prefix, ".sym");
    openFile(&fps.fp[2], f_name, "rb");
    
    sprintf(f_name, "%s%s", f_prefix, ".cnt");
    openFile(&fps.fp[3], f_name, "rb");
    
    sprintf(f_name, "%s%s", f_prefix, ".orn");
    openFile(&fps.fp[4], f_name, "rb");
    FILE *fp;
    openFile(&fp, fmt, "rb");
    fps.ref = new char[fileSizeBytes(fp)/sizeof(char)];
    readFile(fp, fps.ref, fileSizeBytes(fp));
}



// decompress reads
void decompressSeq(std::vector<read_t> &reads, char *ref, dcmp_fps &fps)
{
  const char *mis_sym[64] = {"AAA", "AAC", "AAG", "AAT",
			     "ACA", "ACC", "ACG", "ACT",
			     "AGA", "AGC", "AGG", "AGT",
			     "ATA", "ATC", "ATG", "ATT",
			     "CAA", "CAC", "CAG", "CAT",
			     "CCA", "CCC", "CCG", "CCT",
			     "CGA", "CGC", "CGG", "CGT",
			     "CTA", "CTC", "CTG", "CTT",
			     "GAA", "GAC", "GAG", "GAT",
			     "GCA", "GCC", "GCG", "GCT",
			     "GGA", "GGC", "GGG", "GGT",
			     "GTA", "GTC", "GTG", "GTT",
			     "TAA", "TAC", "TAG", "TAT",
			     "TCA", "TCC", "TCG", "TCT",
			     "TGA", "TGC", "TGG", "TGT",
			     "TTA", "TTC", "TTG", "TTT"}; 
  uint32_t entries = reads.size();
  // read .cnt
  uint8_t *cnt = new uint8_t [entries/sizeof(uint8_t)];
  if (!cnt) {
    printf("Error: unable to allocate memory!\n");
    exit(1);
  }   
  readFile(fps.fp[3], cnt, entries);
  uint32_t tuple_count = 0;
  for (int i = 0; i < entries; i++)
      tuple_count += cnt[i];
  printf("tuple_count: %d\n", tuple_count);
  // read.pos
  uint32_t *pos = new uint32_t [tuple_count*sizeof(uint32_t)];
  if (!pos) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }  
  readFile(fps.fp[0], pos, tuple_count *sizeof(uint32_t));
  printf("Read pos. ");
  // read .len
  uint8_t *len = new uint8_t [tuple_count/sizeof(uint8_t)];
  if (!len) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }  
  readFile(fps.fp[1], len, tuple_count);
  printf("Read len. ");
  // read .sym
  uint8_t *sym = new uint8_t [tuple_count/sizeof(uint8_t)];
  if (!sym) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }  
  readFile(fps.fp[2], sym, tuple_count);
  printf("Read sym. ");

  // read orn
  uint8_t* orn = new uint8_t [entries/sizeof(uint8_t)];
  if (!orn) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }  
  readFile(fps.fp[4], orn, entries);
  printf("Read everything!\n");

  // decompress
  uint32_t n_reads = entries;
  //for (int i = 0; i < 5; i++)
      //printf("%u ", pos[i]);
  for (uint32_t i = 0, k = 0; i < n_reads; i++) {
    int idx = 0;
    //printf("memset.. ");
    memset(reads[i].sym, 0, MAX_SEQ*sizeof(char));
    //printf(" done.\n");
    //printf("%u ", cnt[i]);
    for (uint32_t j = 0; j < cnt[i]; j++) {
      //printf("%s\n", reads[i].sym);
      fflush(stdout);
      uint32_t m = k+cnt[i]-j-1;
      //printf("<(%u) %u %u %u> ", m, pos[m], len[m], sym[m]);
      if (sym[m] == N_SYM) {
	for (int l = 0; l < len[m]; l++) {
	  reads[i].sym[idx++] = 'N';
	}
      }
      else {
	if (sym[m] < 64) {
	  strcat(&reads[i].sym[idx], mis_sym[sym[m]]);
	  idx += 3;
	}
        //printf("i: %u  idx: %d\n", i, idx);
	strncat(&reads[i].sym[idx], &ref[pos[m]], len[m]);
	idx += len[m];
      }
      //printf("\n");
    }
    //printf("compressed single read.\n");
    k += cnt[i];
    reads[i].seq_len = idx;
    if (orn[i] == 1)
      revComp(reads[i].sym, reads[i].seq_len);
    //printf("%u : %s\n", i, reads[i].sym);
  }
}
