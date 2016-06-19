#include "build.hpp"

#define CEIL(a, b) ((a) + (b) - 1) / (b)
#define BUCKET_SIZE 288

// interval structure
struct ival_t {
  uint32_t low;
  uint32_t high;
};

// index structure
struct index_t {
  uint32_t count[64];
  uint8_t bwt[CEIL(BUCKET_SIZE*7,8)];
  uint32_t pad;
};

// sum array
uint32_t sum(uint32_t *a, int n);

// set value in packed bwt
inline void setVal(uint8_t *bwt, uint32_t idx, uint8_t val);

// build index
void build(char *f_prefix, char *ref, uint64_t len)
{
  FILE *fp = NULL;
  int n_threads = omp_get_num_procs();

  // compute suffix array
  printf("computing suffix array ... "); fflush(stdout);
  int64_t *sai = new int64_t [len]; 
  if (!sai) {
    printf("error: unable to allocate suffix array memory!\n");
    exit(1);
  }
  divsufsort64((uint8_t*)ref, sai, (int64_t)len);
  printf("done\n");

  // compute BWT
  printf("computing BWT ... "); fflush(stdout);
  uint8_t *bwt1 = new uint8_t [len];
  uint8_t *bwt2 = new uint8_t [len];
  uint8_t *bwt3 = new uint8_t [len];
  uint8_t *bwt_merge = new uint8_t [len];
  if (!bwt1 || !bwt2 || !bwt3 || !bwt_merge) {
    printf("error: unable to allocate BWT memory!\n");
    exit(1);
  }
#pragma omp parallel for num_threads(n_threads)
  for (uint64_t i = 0; i < len; i++) {
    uint8_t sym_val = 0;

    bwt1[i] = sai[i] > 0 ? ref[sai[i]-1] : ref[len-1+sai[i]];
    bwt2[i] = sai[i] > 1 ? ref[sai[i]-2] : ref[len-2+sai[i]];
    bwt3[i] = sai[i] > 2 ? ref[sai[i]-3] : ref[len-3+sai[i]];

    switch(bwt3[i]) {
    case 'A': sym_val += 0;  break;
    case 'C': sym_val += 16;  break;
    case 'G': sym_val += 32;  break;
    case 'T': sym_val += 48;  break;
    }
    
    switch(bwt2[i]) {
    case 'A': sym_val += 0;  break;
    case 'C': sym_val += 4;  break;
    case 'G': sym_val += 8;  break;
    case 'T': sym_val += 12;  break;
    }

    switch(bwt1[i]) {
    case 'A': sym_val += 0;  break;
    case 'C': sym_val += 1;  break;
    case 'G': sym_val += 2;  break;
    case 'T': sym_val += 3;  break;
    }
    
    if (bwt1[i] == '$' || bwt2[i] == '$' || bwt3[i] == '$') {
      sym_val = 64;
    }

    bwt_merge[i] = sym_val;
  }
  printf("done\n");
 
  // compute counters
  printf("computing counters ... "); fflush(stdout);
  uint32_t count1[5] = {0};
  uint32_t count2[25] = {0};
  uint32_t count3[125] = {0};
 
  for (uint64_t i = 0; i < len; i++) {
    uint8_t count1_val = 0;
    uint8_t count2_val = 0;
    uint8_t count3_val = 0;
    
    switch (bwt3[i]) {
    case '$': count3_val = 0; break;
    case 'A': count3_val = 25; break;
    case 'C': count3_val = 50; break;
    case 'G': count3_val = 75; break;
    case 'T': count3_val = 100; break;
    } 
    
    switch (bwt2[i]) {
    case '$': count3_val += 0;  count2_val = 0; break;
    case 'A': count3_val += 5;  count2_val = 5; break;
    case 'C': count3_val += 10; count2_val = 10; break;
    case 'G': count3_val += 15; count2_val = 15; break;
    case 'T': count3_val += 20; count2_val = 20; break;
    } 

    switch (bwt1[i]) {
    case '$': count3_val += 0; count2_val += 0; count1_val = 0; break;
    case 'A': count3_val += 1; count2_val += 1; count1_val = 1; break;
    case 'C': count3_val += 2; count2_val += 2; count1_val = 2; break;
    case 'G': count3_val += 3; count2_val += 3; count1_val = 3; break;
    case 'T': count3_val += 4; count2_val += 4; count1_val = 4; break;
    } 
    count1[count1_val]++;
    count2[count2_val]++;
    count3[count3_val]++;
  }
  
  // sum counters
  for (int i = 4; i > 0; i--)
    count1[i] = sum(count1, i);
  for (int i = 24; i > 0; i--)
    count2[i] = sum(count2, i);
  for (int i = 124; i > 0; i--)
    count3[i] = sum(count3, i);
  count1[0] = 0;
  count2[0] = 0;
  count3[0] = 0;

  // remove $ counters
  for (int i = 0; i < 4; i++) 
    count1[i] = count1[i+1];
  count1[4] = len;
  for (int i = 0, j = 0; i < 16;) {
    if ((j < 5) || ((j % 5) == 0))
      j++;
    else
      count2[i++] = count2[j++];
  }
  count2[16] = len;
  for (int i = 0, j = 0; i < 64;) {
    if ((j < 25) || ((j % 25) < 5) || ((j % 5) == 0))
      j++;
    else
      count3[i++] = count3[j++];
  }

  // generate suffix array intervals
  ival_t ival1[4];
  ival_t ival2[16];
  for (int i = 0; i < 4; i++) {
    ival1[i].low = count1[i];
    ival1[i].high = count1[i+1]-1;
  }
  for (int i = 0; i < 16; i++) {
    ival2[i].low = count2[i];
    ival2[i].high = sai[count2[i+1]-1] == (int64_t)len-2 ? 
      count2[i+1]-2 : count2[i+1]-1;
  }
  printf("done\n");
  
  // compute FM-index
  printf("generating index ... "); fflush(stdout);
  uint32_t n_buckets = CEIL(len, BUCKET_SIZE);
  index_t *idx = new index_t [n_buckets];
  if (!idx) {
    printf("error: unable to allocate index memory!\n");
    exit(1);
  }
  uint32_t count_tmp[64] = {0};
  memset(idx, 0, n_buckets*sizeof(index_t));
  for (uint64_t i = 0; i < len; i++) { 
    if (i%BUCKET_SIZE==0) {
      for (int j = 0; j < 64; j++) {
	idx[i/BUCKET_SIZE].count[j] = count_tmp[j]+count3[j];
      }
    }
    if (bwt_merge[i] < 64) {
      count_tmp[bwt_merge[i]]+=1;
    }
    setVal(idx[i/BUCKET_SIZE].bwt, i%BUCKET_SIZE, bwt_merge[i]);
  }
  printf("done\n");
  
  // write index to file
  printf("writing index to disk ... "); fflush(stdout);
  char f_name[128];
  sprintf(f_name, "%s%s", f_prefix, ".idx");
  fp = fopen(f_name, "wb");
  if (!fp) {
    printf("error: unable to open file '%s'!\n", f_name);
    exit(1);
  }
  if (fwrite(idx, n_buckets*sizeof(index_t), 1, fp) != 1) {
    printf("error: unable to write to '%s'!\n", f_name);
    exit(1);
  } 
  fclose(fp);
  
  sprintf(f_name, "%s%s", f_prefix, ".1sai");
  fp = fopen(f_name, "wb");
  if (!fp) {
    printf("error: unable to open file '%s'!\n", f_name);
    exit(1);
  } 
  if (fwrite(ival1, 4*sizeof(ival_t), 1, fp) != 1) {
    printf("error: unable to write to '%s'!\n", f_name);
    exit(1);
  } 
  fclose(fp);
  
  sprintf(f_name, "%s%s", f_prefix, ".2sai");
  fp = fopen(f_name, "wb");
  if (!fp) {
    printf("error: unable to open file '%s'!\n", f_name);
    exit(1);
  } 
  if (fwrite(ival2, 16*sizeof(ival_t), 1, fp) != 1) {
    printf("error: unable to write to '%s'!\n", f_name);
    exit(1);
  } 
  fclose(fp);
  
  sprintf(f_name, "%s%s", f_prefix, ".sai");
  fp = fopen(f_name, "wb");
  if (!fp) {
    printf("error: unable to open file '%s'!\n", f_name);
    exit(1);
  } 
  uint32_t *sai_tmp = NULL;
  sai_tmp = new uint32_t [len];
  if (!sai_tmp) {
    printf("error: unable to allocate suffix array memory!\n");
    exit(1);
  }
#pragma omp parallel for num_threads(n_threads)
  for (uint64_t i = 0; i < len; i++) {
    sai_tmp[i] = sai[i];
  } 
  if (fwrite(sai_tmp, len*sizeof(uint32_t), 1, fp) != 1) {
    printf("error: unable to write to '%s'!\n", f_name);
    exit(1);
  } 
  fclose(fp);
  printf("done\n");
  
  // cleanup
  delete[] idx;
  delete[] sai;
  delete[] sai_tmp;
  delete[] bwt1;
  delete[] bwt2;
  delete[] bwt3;
  delete[] bwt_merge;
}

// sum array
uint32_t sum(uint32_t *a, int n)
{
  uint32_t s = 0;
  
  for (int i = 0; i < n; i++)
    s += a[i];
  
  return s;
}

// set value in packed bwt
inline void setVal(uint8_t *bwt, uint32_t idx, uint8_t val)
{
  uint16_t tmp = val << ((idx*7)%8);
  bwt[(idx*7)/8] |= (uint8_t) tmp;
  bwt[((idx*7)/8)+1] |= (uint8_t) (tmp>>8);
}
