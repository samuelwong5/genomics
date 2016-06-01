/* main.cpp ----- James Arram 2016 */

#include "def.hpp"
#include "file.hpp"
#include "inc.hpp"
#include "reads.hpp"
#include "index.hpp"
#include "cmp_sw.hpp"

int main(int argc, char *argv[]) {
  
  FILE *fp = NULL;
  index_t *idx = NULL;
  uint32_t *sai = NULL;
  uint64_t sa_map_size;
  ival_t ival1[4];
  ival_t ival2[16];
  char f_name[128];
  std::thread thr;
  struct timeval  tv1, tv2;
 
  // check usage
  if (argc != 3) {
    printf("usage: %s <fmt file> <fastq file>\n", argv[0]);
    exit(1);
  }

  printf("loading index data ... "); fflush(stdout);
  gettimeofday(&tv1, NULL);

  // load index
  sprintf(f_name, "%s%s", argv[1], ".idx");
  openFile(&fp, f_name, "rb");
  uint64_t index_bytes = fileSizeBytes(fp);
  idx = new index_t [index_bytes/sizeof(index_t)];
  if (!idx) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }  
  readFile(fp, idx, index_bytes);
  fclose(fp);

  // load 1-step intervals
  sprintf(f_name, "%s%s", argv[1], ".1sai");
  openFile(&fp, f_name, "rb");
  readFile(fp, ival1, 4*sizeof(ival_t));
  fclose(fp);
  
  // load 2-step intervals
  sprintf(f_name, "%s%s", argv[1], ".2sai");
  openFile(&fp, f_name, "rb");
  readFile(fp, ival2, 16*sizeof(ival_t));
  fclose(fp);

  // load suffix array
  sprintf(f_name, "%s%s", argv[1], ".sai");
  mapSuffixArray(f_name, &sai, &sa_map_size);

  gettimeofday(&tv2, NULL);
  printf("OK [%.2f s]\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	 (double) (tv2.tv_sec - tv1.tv_sec));

  // allocate input buffer
  char *in_buff = new char [BUFF_SIZE+512];
  if (!in_buff) {
    printf("error: unable to allocate memory!\n");
    exit(1);
  }

  std::vector<read_t> r0, r1;
  r0.reserve(CEIL(BUFF_SIZE, SEQ_LEN*2));
  r1.reserve(CEIL(BUFF_SIZE, SEQ_LEN*2));

  printf("compressing reads ... \n"); fflush(stdout);

  // read first batch
  openFile(&fp, argv[2], "r");
  uint64_t len = fileSizeBytes(fp);
  uint64_t bytes_r = 0;
  uint64_t size =  bytes_r + BUFF_SIZE <= len ? BUFF_SIZE : len - bytes_r;
  loadReads(fp, r0, in_buff, size, true, &bytes_r);
  
  // process batches
  uint32_t cnt = 0;
  for (int i = 0; ; i++) {
    bool r_ctrl = bytes_r < len ? true : false;
    size = bytes_r + BUFF_SIZE <= len ? BUFF_SIZE : len - bytes_r; 
    
    // read to r1, process r0
    if (!(i%2)) {
      thr = std::thread(loadReads, fp, std::ref(r1), in_buff, size, r_ctrl, &bytes_r);
      if (r0.size() > 0) {
	cnt += r0.size();
	compress(r0, idx, ival1, ival2, sai, argv[2]);
	// compress meta
	// compress quality scores
      }
      else
	break;
    }
    
    // read to r0, process r1
    else {
      thr = std::thread(loadReads, fp, std::ref(r0), in_buff, size, r_ctrl, &bytes_r);
      if (r1.size() > 0) {
	cnt += r1.size();
	compress(r1, idx, ival1, ival2, sai, argv[2]);
	// compress meta
	// compress quality scores	
      }
      else 
	break;
    }
    thr.join();
    printf("processed %u reads\n", cnt);
  }
  thr.join();
 
  if (munmap(sai, sa_map_size) == -1) {
    printf("error: unable to unmap file!\n");
    exit(1);
  }

  delete[] idx;
  delete[] in_buff;
 
  return 0;
}