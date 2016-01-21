#ifndef LZW_H
#define LZW_H

#include <string.h>
#include "bits.h"

typedef struct lzw_dict_entry
{
    struct lzw_dict_entry *next;
    char *seq;
    int code;
    int length;
} lzw_dict_entry;

typedef struct lzw_dict
{
    lzw_dict_entry *head;
    lzw_dict_entry *tail;
    int count;
    int bits;
    int next_bit;
} lzw_dict;


lzw_dict * lzw_dict_init(char *);
void lzw_dict_add(lzw_dict *, char *, int);
void lzw_dict_free(lzw_dict *);
char * lzw_encode_run(lzw_dict *, char *, data *);
void lzw_dict_print(lzw_dict *);
data * lzw_encode(char *, char *);
char * lzw_dict_get(lzw_dict *dict, int code);
char * lzw_decode(char *alphabet, data *d);
int lzw_benchmark(char *);

#endif