/* def.hpp ----- James Arram 2016 */

#ifndef DEF_HPP
#define DEF_HPP

// constants used in program
#define SEQ_LEN 100
#define MAX_SEQ 150
#define MAX_META 128
#define N_DFE 2 // support up to 8 DFEs
#define N_THREADS 16
#define BUFF_SIZE 400000000 // disk IO buffer size
#define N_SYM 65 // must be > 64
#define BUCKET_SIZE 288 

// round up division
#define CEIL(a, b) (((a)+(b)-1)/(b))

#endif
