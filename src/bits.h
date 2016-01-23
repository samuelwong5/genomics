#ifndef BITS_H
#define BITS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Utility functions to writing individual bits into a buffer to
 * increase compression rates. Uses a uint32_t buffer so a maximum
 * of 32bits can be written or read at the same time. 
 */

typedef struct data
{
    uint32_t *head;         // Pointer to the head of the block of memory
    uint32_t *curr;         // Pointer to next write
    uint8_t offset;         // Bit offset of write pointer
    uint32_t *read_curr;    // Pointer to next read
    uint8_t read_offset;    // Bit offset of read pointer
    uint8_t size;           // Size of allocated memory
    uint8_t index;          // Size of used memory
} data;

void bits_write(data *, uint32_t, uint8_t);
uint32_t bits_read(data *, uint8_t);
void bits_print(data *);
data * data_init();
void data_free(data *);
void data_expand(data *);
int data_end(data *);
void p32(uint32_t b);
int data_size(data *d);

#endif