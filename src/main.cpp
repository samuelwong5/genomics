#include <iostream>
#include "metadata/metadataencoder.hpp"
#include "sequence/def.hpp"
#include "sequence/file.hpp"
#include "sequence/inc.hpp"
#include "sequence/reads.hpp"
#include "sequence/index.hpp"
#include "sequence/cmp_seq.hpp"
#include "quality/qualityscoreencoder.hpp"

void decompress(char *);
void compress(char **);

void writeReads(std::vector<read_t>& reads, std::ofstream& of)
{
    for (auto it = reads.begin(); it != reads.end(); it++)
    {
        of << it->meta_data << "\n";
        of << it->sym << "\n";
        of << "+\n";
        of << it->q_score << "\n";
    }
   // if (reads.size() > 0) {
   //   std::cout << reads[0].meta_data << "\n" << reads[0].sym << "\n" << reads[0].q_score << "\n";
   //   std::cout << reads[reads.size()-1].meta_data << "\n" << reads[reads.size()-1].sym << "\n" << reads[reads.size()-1].q_score << "\n";
   // }
}

void decompress(char *filename, char * fmt)
{
  printf("Decompressing bzip...\n");
  char syscmd[120];
  sprintf(syscmd, "tar xvf %s.tar.bz2 --absolute-names", filename);
  system(syscmd);
 // printf(syscmd);
  sprintf(syscmd, "pbzip2 -d %s.*", filename);
  system(syscmd);
  QualityScoreEncoder qse(filename);  
  MetaDataEncoder mde(filename);
  std::vector<read_t> r0;
  std::vector<read_t> r1;
  bool is_end = false;
  std::string ofilename(filename);
  ofilename.append(".original");
  std::ofstream of(ofilename);
  dcmp_fps dfps;
  decompress_init(dfps, filename, fmt);
  int batch_number = 1;
  std::thread thr;
  while (!is_end) 
  {
      printf("Decompressing batch %d\n", batch_number);
      if (batch_number % 2 == 0)
      {   
          thr = std::thread(writeReads, std::ref(r1), std::ref(of));
          std::cout << "Decompressing metadata\n";
          is_end = mde.metadata_decompress(r0, filename);
          if (is_end) { break; }
          std::cout << "Decompressing sequence\n";
          decompressSeq(r0, dfps.ref, dfps);
          std::cout << "Decompressing qs\n";
          qse.qualityscore_decompress(r0, filename);   
      } 
      else
      {
          thr = std::thread(writeReads, std::ref(r0), std::ref(of));
          std::cout << "Decompressing metadata\n";
          is_end = mde.metadata_decompress(r1, filename);
          if (is_end) { break; }
          std::cout << "Decompressing sequence\n";
          decompressSeq(r1, dfps.ref, dfps);
          std::cout << "Decompressing qs\n";
          qse.qualityscore_decompress(r1, filename);
      }
      batch_number++;
      thr.join();
  }
  thr.join();
  for (int i = 0; i < 5; i++)
      fclose(dfps.fp[i]);
  of.close();
}

int main(int argc, char *argv[]) {
  // check usage
  if (argc < 3) {
    printf("Usage: %s [--compress|--decompress] <fastq file> <idx>\n", argv[0]);
    exit(1);
  }
  
  if (strncmp(argv[1], "--compress", 10) == 0)
      compress(argv);      
  else if (strncmp(argv[1], "--decompress", 12) == 0)
      decompress(argv[2], argv[3]);
  else
  {
      printf("Unknown flag: %s\n", argv[1]);
  }
}

void compress(char** argv)
{
  FILE *fp = NULL;
  index_t *idx = NULL;
  uint32_t *sai = NULL;
  uint64_t sa_map_size;
  ival_t ival1[4];
  ival_t ival2[16];
  char f_name[128];
  std::thread thr;
  struct timeval  tv1, tv2;
  
  printf("Loading FM-index...\n");
  gettimeofday(&tv1, NULL);
  
  //load index
  sprintf(f_name, "%s%s", argv[3], ".idx");
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
  sprintf(f_name, "%s%s", argv[3], ".1sai");
  openFile(&fp, f_name, "rb");
  readFile(fp, ival1, 4*sizeof(ival_t));
  fclose(fp);
  
  // load 2-step intervals
  sprintf(f_name, "%s%s", argv[3], ".2sai");
  openFile(&fp, f_name, "rb");
  readFile(fp, ival2, 16*sizeof(ival_t));
  fclose(fp);

  // load suffix array
  sprintf(f_name, "%s%s", argv[3], ".sai");
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
  gettimeofday(&tv1, NULL);
  
  
  // read first batch
  openFile(&fp, argv[2], "r");
  uint64_t len = fileSizeBytes(fp);
  uint64_t bytes_r = 0;
  uint64_t size =  bytes_r + BUFF_SIZE <= len ? BUFF_SIZE : len - bytes_r;
  uint64_t cnt = 0;
  uint8_t flags = 0; // last bit set if + strand; 2nd last bit set if _NOT_ repeated metadata
  MetaDataEncoder mde;
  QualityScoreEncoder qse;
  loadReads(fp, r0, in_buff, size, true, &bytes_r, &flags);
  // process batches
  std::cout << "BEGIN COMPRESSION...\n";
  for (int i = 0; ; i++) {
    bool r_ctrl = bytes_r < len ? true : false;
    size = bytes_r + BUFF_SIZE <= len ? BUFF_SIZE : len - bytes_r; 
    printf("Compressing Batch: %d [Entries: %d]\n", i, i%2==0 ? r0.size() : r1.size());   
    // read to r1, proess r0
    if (i == 0)
    {
        std::string strand_file(argv[2]);
        strand_file.append(".str");
        std::ofstream ofs(strand_file);
        ofs << flags;
        ofs.close();
    }
    if (!(i%2)) {
      thr = std::thread(loadReads, fp, std::ref(r1), in_buff, size, r_ctrl, &bytes_r, &flags);
      if (r0.size() > 0) {
	    cnt += r0.size();
            printf(" [SEQUENCE]\n");
            compressSeq(r0, idx, ival1, ival2, sai, argv[2]);
	    // compress meta
	    mde.metadata_compress(r0, argv[2]);
            // compress quality scores
            qse.qualityscore_compress(r0, argv[2]);
      }
      else
	    break;
    }
    
    // read to r0, process r1
    else {
      thr = std::thread(loadReads, fp, std::ref(r0), in_buff, size, r_ctrl, &bytes_r, &flags);
      if (r1.size() > 0) {
	    cnt += r1.size();
            printf(" [SEQUENCE]\n");
	    compressSeq(r1, idx, ival1, ival2, sai, argv[2]);
	    // compress meta
	    mde.metadata_compress(r1, argv[2]);
             // compress quality scores	
            qse.qualityscore_compress(r1, argv[2]);
      }
      else 
	    break;
    }
    thr.join();
    printf(" (Total: %llu reads.)\n", cnt);
  }
  thr.join();
  gettimeofday(&tv2, NULL);
  printf("Compress total time [%.2f s]\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
  if (munmap(sai, sa_map_size) == -1) {
    printf("Error: unable to unmap file!\n");
    exit(1);
  }
  printf("Passing through pbzip...\n");
  char syscmd[120];
  char filename[120];
  sprintf(syscmd, "pbzip2 %s.*", argv[2]);
  system(syscmd);
  printf("Tar...\n");
  strncpy(filename, argv[2], strlen(argv[2])-5);
  sprintf(syscmd, "tar cvf %stemp %s.* --absolute-names", filename, argv[2]);
  system(syscmd);
  printf("Cleaning up...\n");
  sprintf(syscmd, "rm %s.*", argv[2]);
  system(syscmd);
  sprintf(syscmd, "cp %stemp %s.tar.bz2", filename, argv[2]);
  system(syscmd);
  printf("COMPRESSION FINISHED!\n");

  delete[] idx;
  delete[] in_buff;
}
