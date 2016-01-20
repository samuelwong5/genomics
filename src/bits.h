#ifndef BITS_H
#define BITS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct data
{
    uint32_t *head;
    uint32_t *curr;
    uint8_t offset;
    uint8_t size;
    uint8_t index;
} data;

void bits_write(data *, uint32_t, uint8_t);
uint32_t bits_read(data *, uint8_t);
void bits_print(data *);
data * data_init();
void data_free(data *);
void data_expand(data *);
void p32(uint32_t b);

#endif