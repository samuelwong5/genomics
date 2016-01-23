#ifndef NAIVE_RL_H
#define NAIVE_RL_H

/*
 *  Simple implementation of run-length encoding.
 *  1. Get the first character c of the input string 
 *  2. Count the consecutive occurences n of the character c
 *  3. Write c (8 bits) and n (8 bits) into the buffer
 *  4. Move the pointer to the character after the last occurence
 *  5. Repeat 1-4 until terminating character
 *  
 *  Resulting encoded buffer:
 *  0        8        16       24       32
 *  |--------|--------|--------|--------|-- ... 
 *  |   c1   |   n1   |   c2   |   n2   | 
 *  |--------|--------|--------|--------|-- ... 
 */

#include <stdlib.h>
#include <string.h>

char * naive_rl_encode_run(char *, char *);
char * naive_rl_encode(char *);
char * naive_rl_decode(char *);
int naive_rl_benchmark(char *);

#endif