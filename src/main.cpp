#include <iostream>

#include "sequence/def.hpp"
#include "sequence/file.hpp"
#include "sequence/inc.hpp"
#include "sequence/reads.hpp"
#include "sequence/index.hpp"
#include "sequence/cmp_seq.hpp"
#include "metadata/metadataencoder.hpp"
#include "quality/qualityscoreencoder.hpp"

void writeReads(std::vector<read_t>&, std::ofstream&, uint8_t);
void decompress(char *, char *);
void compress(char **);

const int FILE_EXT_COUNT = 8;
const char * const FILE_EXT[] = {".cnt", ".len", ".md", ".orn", ".pos", ".qs", ".str", ".sym"};

void 
writeReads(std::vector<read_t>& reads, std::ofstream& of, uint8_t flags)
{
    for (auto it = reads.begin(); it != reads.end(); it++)
    {
        // Metadata
        of << it->meta_data << "\n";
        
        // Sequence
        of << it->sym << "\n";
        
        // Strand
        if (flags & 0x1)
        of << "+";
        else
        of << "-";
        if ((flags & 0x2) == 0)
        of << it->meta_data;
        of << "\n";
        
        // Quality scores
        of << it->q_score << "\n";
    }
}


void 
decompress(char *filename, char * fmt)
{
    printf("\nSTARTING DECOMPRESSION                  [%.2f s]\n", 0.f);
    printf("  - bzip decompression");
    char syscmd[120];
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
    if (strncmp(filename + strlen(filename) - 8, ".tar.bz2", 8) == 0)
    sprintf(syscmd, "tar xf %s --absolute-names", filename);  
    else
    sprintf(syscmd, "tar xf %s.tar.bz2 --absolute-names", filename);
    system(syscmd);
    for (int i = 0; i < FILE_EXT_COUNT; i++)
    {
        sprintf(syscmd, "pbzip2 -d %s%s.bz2", filename, FILE_EXT[i]);
        system(syscmd);
    }
    gettimeofday(&tv2, NULL);
    printf("                  [%.2f s]\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
    (double) (tv2.tv_sec - tv1.tv_sec));
    
    uint8_t str_flags;
    {   // Get strand flags
        std::string strand_file(filename);
        strand_file.append(".str");
        std::ifstream str_if(strand_file);
        std::string fl;
        std::getline(str_if, fl);
        str_flags = atoi(fl.c_str());
    }
    
    QualityScoreEncoder qse(filename);  
    MetaDataEncoder mde(filename);
    std::vector<read_t> r0;
    std::vector<read_t> r1;
    std::string ofilename(filename);
    ofilename.append(".original");
    std::ofstream of(ofilename);
    dcmp_fps dfps;
    decompress_init(dfps, filename, fmt);
    int batch_number = 1;
    std::thread thr;
    uint32_t reads = 0;
    gettimeofday(&tv1, NULL);
    bool is_end = false;
    while (!is_end) 
    {
        if (batch_number % 2 == 0)
        {   
            thr = std::thread(writeReads, std::ref(r1), std::ref(of), str_flags);
            printf("\r  - Decompressing batch %d [METADATA]", batch_number); fflush(stdout);
            is_end = mde.metadata_decompress(r0, filename);
            if (r0.size() == 0) break;
            printf("\r  - Decompressing batch %d [SEQUENCE]", batch_number); fflush(stdout);
            decompressSeq(r0, dfps.ref, dfps);
            printf("\r  - Decompressing batch %d [Q. SCORE]", batch_number); fflush(stdout);
            qse.qualityscore_decompress(r0, filename);   
            reads += r0.size();
        } 
        else
        {
            thr = std::thread(writeReads, std::ref(r0), std::ref(of), str_flags);
            printf("\r  - Decompressing batch %d [METADATA]", batch_number); fflush(stdout);
            is_end = mde.metadata_decompress(r1, filename);
            if (r1.size() == 0) break;
            printf("\r  - Decompressing batch %d [SEQUENCE]", batch_number); fflush(stdout);
            decompressSeq(r1, dfps.ref, dfps);
            printf("\r  - Decompressing batch %d [Q. SCORE]", batch_number); fflush(stdout);
            qse.qualityscore_decompress(r1, filename);   
            reads += r1.size();
        }
        printf("    (Total reads: %d)\n", reads);
        batch_number++;
        thr.join();
    }
    
    if (batch_number % 2 == 0)
    writeReads(r1, of, str_flags);
    else
    writeReads(r0, of, str_flags);
    
    // Cleanup
    if (thr.joinable())
    thr.join();
    for (int i = 0; i < 5; i++)
    fclose(dfps.fp[i]);
    of.close();
    
    gettimeofday(&tv2, NULL);
    for (int i = 0; i < FILE_EXT_COUNT; i++)
    {
        sprintf(syscmd, "rm %s%s", filename, FILE_EXT[i]);
        system(syscmd);
    }
    
    printf("\rFINISHED DECOMPRESSION                  [%.2f s]\n", 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
}



int 
main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [--compress|--decompress] <fastq file> <index file>\n", argv[0]);
        exit(1);
    }
    
    if (strncmp(argv[1], "--compress", 10) == 0)
    compress(argv);      
    else if (strncmp(argv[1], "--decompress", 12) == 0)
    decompress(argv[2], argv[3]);
    else
    {
        printf("Unknown flag: %s\n", argv[1]);
        printf("Usage: %s [--compress|--decompress] <fastq file> <index file>\n", argv[0]);
    }
}

void 
compress(char** argv)
{
    FILE *fp = NULL;
    index_t *idx = NULL;
    uint32_t *sai = NULL;
    uint64_t sa_map_size;
    ival_t ival1[4];
    ival_t ival2[16];
    char f_name[128];
    std::thread thr;
    struct timeval  tv1, tv2, start;
    gettimeofday(&start, NULL);
    
    printf("INITIALIZING\n");
    printf("  - Loading FM-index         ");
    gettimeofday(&tv1, NULL);
    
    //load index
    sprintf(f_name, "%s%s", argv[3], ".idx");
    openFile(&fp, f_name, "rb");
    uint64_t index_bytes = fileSizeBytes(fp);
    idx = new index_t [index_bytes/sizeof(index_t)];
    if (!idx) {
        printf("\n[FATAL] Unable to allocate memory for index!\n");
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
    printf("[%.2f s]\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
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
    std::cout << "BEGIN COMPRESSION... [" << N_THREADS << " threads]" << std::endl;
    loadReads(fp, r0, in_buff, size, true, &bytes_r, &flags);
    
    
    // process batches
    double seq_time = 0.f, meta_time = 0.f, qs_time = 0.f;
    printf("|BATCH| ENTRIES | TOT ENTRIES |  SEQ.  |  MET.  |  QSC.  | TOTAL TIME |\n");
    for (int i = 0; ; i++) {
        bool r_ctrl = bytes_r < len ? true : false;
        size = bytes_r + BUFF_SIZE <= len ? BUFF_SIZE : len - bytes_r; 
        if (i % 2 == 0)
        cnt += r0.size();
        else
        cnt += r1.size();
        
        printf("|%5d|%9lu|%13lu|", i + 1, 
        i%2==0 ? r0.size() : r1.size(),cnt); fflush(stdout);
        // read to r1, proess r0
        if (i == 0)
        {
            std::string strand_file(argv[2]);
            strand_file.append(".str");
            std::ofstream ofs(strand_file);
            ofs << (uint32_t) flags;
            ofs.close();
        }
        if (!(i%2)) {
            thr = std::thread(loadReads, fp, std::ref(r1), in_buff, size, r_ctrl, &bytes_r, &flags);
            if (r0.size() > 0) {
                struct timeval t1, t2;
                gettimeofday(&t1, NULL);
                compressSeq(r0, idx, ival1, ival2, sai, argv[2]);
                gettimeofday(&t2, NULL);
                double d_seq_time = (double) (t2.tv_usec - t1.tv_usec) / 1000000 + (double) (t2.tv_sec - t1.tv_sec);
                printf("%7.1fs|", d_seq_time); fflush(stdout);
                seq_time += d_seq_time;
                // compress meta
                mde.metadata_compress(r0, argv[2]);
                gettimeofday(&t1, NULL);
                double d_meta_time = (double) (t1.tv_usec - t2.tv_usec) / 1000000 + (double) (t1.tv_sec - t2.tv_sec); 
                printf("%7.1fs|", d_meta_time); fflush(stdout);
                meta_time += d_meta_time;
                // compress quality scores
                qse.qualityscore_compress(r0, argv[2]);
                gettimeofday(&t2, NULL);
                double d_qs_time = (double) (t2.tv_usec - t1.tv_usec) / 1000000 + (double) (t2.tv_sec - t1.tv_sec);
                qs_time += d_qs_time;
                printf("%7.1fs|%11.1fs|\n", d_meta_time, seq_time+meta_time+qs_time); fflush(stdout);
               
            }
            else
            break;
        }
        // read to r0, process r1
        else {
            thr = std::thread(loadReads, fp, std::ref(r0), in_buff, size, r_ctrl, &bytes_r, &flags);
            if (r1.size() > 0) {
                struct timeval t1, t2;
                gettimeofday(&t1, NULL);
                compressSeq(r1, idx, ival1, ival2, sai, argv[2]);
                gettimeofday(&t2, NULL);
                double d_seq_time = (double) (t2.tv_usec - t1.tv_usec) / 1000000 + (double) (t2.tv_sec - t1.tv_sec);
                printf("%7.1fs|", d_seq_time); fflush(stdout);
                seq_time += d_seq_time;
                // compress meta
                mde.metadata_compress(r1, argv[2]);
                gettimeofday(&t1, NULL);
                double d_meta_time = (double) (t1.tv_usec - t2.tv_usec) / 1000000 + (double) (t1.tv_sec - t2.tv_sec); 
                printf("%7.1fs|", d_meta_time); fflush(stdout);
                meta_time += d_meta_time;
                // compress quality scores	
                qse.qualityscore_compress(r1, argv[2]);
                gettimeofday(&t2, NULL);
                double d_qs_time = (double) (t2.tv_usec - t1.tv_usec) / 1000000 + (double) (t2.tv_sec - t1.tv_sec);
                qs_time += d_qs_time;
                printf("%7.1fs|%11.1fs|\n", d_meta_time, seq_time+meta_time+qs_time); fflush(stdout);
            }
            else 
            break;
        }
        thr.join();
    }
    thr.join();
    gettimeofday(&tv2, NULL);
    printf("\n - Main compression done     [%.2f s]\n", seq_time+meta_time+qs_time);
    if (munmap(sai, sa_map_size) == -1) {
        printf("[FATAL] Unable to unmap reference file!\n");
        exit(1);
    }
    
    
    char syscmd[3000];
    char filename[500];
    
    printf(" - Compressing bzip          ");
    for (int i = 0; i < FILE_EXT_COUNT; i++)
    {
        sprintf(syscmd, "pbzip2 %s%s", argv[2], FILE_EXT[i]);
        system(syscmd);
    }
    gettimeofday(&tv1, NULL);
    printf("[%.2f s]\n", (double) (tv1.tv_usec - tv2.tv_usec) / 1000000 + (double) (tv1.tv_sec - tv2.tv_sec));
    
    
    printf(" - Archiving                 ");
    strncpy(filename, argv[2], strlen(argv[2])-5); // removing '.fastq'
    sprintf(syscmd, "tar cf %stemp ", filename);
    for (int i = 0; i < FILE_EXT_COUNT; i++)
    {
        char fn[500];
        sprintf(fn, "%s%s.bz2 ", argv[2], FILE_EXT[i]);
        strcat(syscmd, fn);
    }
    strcat(syscmd, "--absolute-names");
    system(syscmd);
    gettimeofday(&tv2, NULL);
    printf("[%.2f s]\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
    
    for (int i = 0; i < FILE_EXT_COUNT; i++)
    {
        sprintf(syscmd, "rm %s%s.bz2", argv[2], FILE_EXT[i]);
        system(syscmd);
    }
    
    sprintf(syscmd, "mv %stemp %s.tar.bz2", filename, argv[2]);
    system(syscmd);
    
    printf("COMPRESSION FINISHED         ");
    gettimeofday(&tv1, NULL);
    printf("[%.2f s]\n", (double) (tv1.tv_usec - start.tv_usec) / 1000000 + (double) (tv1.tv_sec - start.tv_sec));
    
    delete[] idx;
    delete[] in_buff;
}
