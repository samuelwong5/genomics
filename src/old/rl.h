#ifndef RL_H
#define RL_H

#include <math.h>
#include "bits.h"

/*
*  Implementation of run-length encoding.
*  1. Get the first character c of the input string
*  2. Count the consecutive occurences n of the character c
*  3. Write c (CHAR_LENGTH bits) and n (COUNT_LENGTH bits) into the buffer
*  4. Move the pointer to the character after the last occurence
*  5. Repeat 1-4 until terminating character
*
*  CHAR_LENGTH is calculated based on the alphabet size of the plaintext:
*    - CHAR_LENGTH = ceil(log_2(|alphabet| + 1))
*  which is the minimum number of bits to have a bijective mapping from
*  the alphabet and a stopcode (0x0).
*
*  COUNT_LENGTH is predefined (should be enough to store almost all of the run lengths)
*
*  Sample resulting encoded buffer:
*    - Alphabet: a-zA-Z (52 characters + 1 stopcode) -> CHAR_LENGTH = 6
*    - COUNT_LENGTH = 4 
*  0     6    10    16   20    26   30
*  |-----|----|-----|----|-----|----|-- ..
*  |  c1 | n1 |  c2 | n2 |  c3 | n3 |
*  |-----|----|-----|----|-----|----|-- ..
*/

void rl_init(char *);
char * rl_encode_run(char *, data *);
data * rl_encode(char *, char *);
char * rl_decode(char *, data *);
int rl_benchmark(char *);

#endif