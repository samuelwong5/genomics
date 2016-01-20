#include "bits.h"

/*
ARGUMENTS:

DATA - the struct containing the buffer where data is written
     offset
0    |   32       128
|XXXX----|--------|

VALUE - the value to be written into the buffer

LENGTH - the number of bits that should be used to contain the value
     offset
0    |   32       128
|XXXXVVVV|VVVV----|
             |
			 offset + length

*/

const int BUFFER_SIZE = 32;
const int DATA_INIT_SIZE = 64;

void bits_write(data *d, uint32_t value, uint8_t length)
{
	if (length + d->offset < BUFFER_SIZE) { // Fits in buffer[0]
		uint32_t mask = 0xffffffff;
		mask >>= (BUFFER_SIZE - length);
		mask &= value;
		*d->curr |= (mask << (BUFFER_SIZE - d->offset - length));
	} else {
		uint32_t mask = 0xffffffff;
		mask >>= d->offset;
		uint32_t rvalue = value >> (length - (BUFFER_SIZE - d->offset));
		rvalue &= mask;
		*d->curr |= rvalue;
		d->curr++;
		d->index++;
		uint32_t lvalue = value << (2 * BUFFER_SIZE - length - d->offset);
		*d->curr |= lvalue;
	}
	d->offset = (d->offset + length) % BUFFER_SIZE;
	// data_expand(d);
}

uint32_t bits_read(data *d, uint8_t length)
{
	uint32_t value = 0xffffffff;
	if (length + d->offset < BUFFER_SIZE) {
		value >>= (BUFFER_SIZE - length);
		value <<= (BUFFER_SIZE - d->offset - length);
		value &= *d->curr;
		value >>= (BUFFER_SIZE - d->offset - length);
	} else {
		uint32_t rmask = 0xffffffff;
		rmask >>= (d->offset);
		rmask &= *d->curr;
		rmask <<= (length + d->offset - BUFFER_SIZE);
		d->curr++;
		d->index++;
		uint32_t lmask = 0xffffffff;
		lmask <<= (2 * BUFFER_SIZE - length - d->offset);
		lmask &= *d->curr;
		lmask >>= (2 * BUFFER_SIZE - length - d->offset);
		value = rmask | lmask;
	}
	d->offset = (d->offset + length) % BUFFER_SIZE;
	return value;
}

/************************
* DEBUG BITSTRING UTILS *
*************************/
void bits_print(data *d)
{
	int j = 0;
	int len = d->index + 1;
	for (; j < len; ++j) {
		uint32_t b = d->head[j];
		uint32_t mask = 1 << (BUFFER_SIZE - 1);
		int i = 0;
		if (j < len - 1) {
			printf("%d: ", j);
			for (; i < BUFFER_SIZE; ++i) {
				printf("%u", b&mask ? 1 : 0);
				b <<= 1;
			}
		} else if (d->offset != 0) {
			printf("%d: ", j);
			for (; i < d->offset; ++i) {
				printf("%u", b&mask ? 1 : 0);
				b <<= 1;
			}
		}
		printf("\n");
	}
	printf("\n");
}

void p32(uint32_t b)
{
	uint32_t mask = 1 << (BUFFER_SIZE - 1);
	int i = 0;
	for (; i < BUFFER_SIZE; ++i) {
		printf("%u", b&mask ? 1 : 0);
		b <<= 1;
	}
	printf("\n");
}


/*********************
* DATA STRUCT UTILS *
*********************/
data * data_init()
{
	data *d = (data *) malloc(sizeof(data));
	uint32_t *buffer = (uint32_t *) calloc(DATA_INIT_SIZE, BUFFER_SIZE / 4);
	d->head = buffer;
	d->curr = buffer;
	d->offset = 0;
	d->index = 0;
	d->size = DATA_INIT_SIZE;
	return d;
}

void data_free(data *d)
{
	free(d->head);
	free(d);
}

void data_expand(data *d)
{
	// Double the buffer length if near capacity
	if (d->index >= 0.9 * d->size) {
		uint32_t *new_buffer = (uint32_t *) realloc(d->head, d->size * 2 * BUFFER_SIZE / 8);
		d->head = new_buffer;
		d->curr = new_buffer += d->index;
		memset(d->head[(int) d->size], 0, d->size * BUFFER_SIZE / 8);
		d->size *= 2;
	}
}