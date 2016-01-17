#ifndef BITS_H
#define BITS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct data
{
	uint32_t *head;
	uint32_t *curr;
	uint8_t offset;
	uint8_t size;
} data;

const int BUFFER_SIZE = 32;

void bits_write(data *, uint32_t, uint8_t);
void bits_print(data *);
data * data_init(uint8_t size);
void data_free(data *);

#endif