#include "bits.h"

int main() 
{
	data *d = data_init((uint8_t) 4);
	bits_write(d, 1, 1);
	bits_write(d, 1, 2);
	bits_write(d, 1, 3);
	bits_write(d, 1, 32);
	bits_print(d);
}

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
		uint32_t lvalue = value << (2 * BUFFER_SIZE - length - d->offset);
		*d->curr |= lvalue;
	}
	d->offset = (d->offset + length) % BUFFER_SIZE;
}

void bits_print(data *d)
{
	int j = 0;
	int len = d->size;
	for (; j < len; ++j) {
		uint32_t b = d->head[j];
		uint32_t mask = 1 << (BUFFER_SIZE - 1);
		printf("%d: ", j);
		int i = 0;
		for (; i < BUFFER_SIZE; ++i) {
			printf("%u", b&mask ? 1 : 0);
			b <<= 1;
		}
		printf("\n");
	}
	printf("\n");
}


/*********************
* DATA STRUCT UTILS *
*********************/
data * data_init(uint8_t size)
{
	data *d = (data *)malloc(sizeof(data));
	uint32_t *buffer = (uint32_t *)calloc(size, BUFFER_SIZE / 4);
	d->head = buffer;
	d->curr = buffer;
	d->offset = 0;
	d->size = size;
	return d;
}

void data_free(data *d)
{
	free(d->head);
	free(d);
}
