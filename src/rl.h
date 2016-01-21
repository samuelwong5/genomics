#ifndef RL_H
#define RL_H

#include <math.h>
#include "bits.h"

char * rl_encode_run(char *, data *);
char * rl_decode(char *, data *);
data * rl_encode(char *, char *);
void rl_init(char *);
int rl_benchmark(char *);

#endif