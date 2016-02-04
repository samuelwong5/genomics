#include "bitbuffer.hpp"

const int BUFFER_SIZE = 32;
const int DATA_INIT_SIZE = 64;

BitBuffer::BitBuffer(void) 
{
    buffer = std::vector<uint32_t> (DATA_INIT_SIZE, 0);
    write_it = buffer.begin();
    write_offset = 0;
    write_index = 0;
    read_it = buffer.begin();
    read_offset = 0;
    read_index = 0;             
    alloc_size = DATA_INIT_SIZE;     
}

BitBuffer::~BitBuffer(void)
{

}

void BitBuffer::write(uint32_t value, uint8_t length)
{
    if (length + write_offset <= BUFFER_SIZE) { // Fits in buffer[0]
        uint32_t mask = 0xffffffff;
        mask >>= (BUFFER_SIZE - length);
        mask &= value;
        *write_it |= (mask << (BUFFER_SIZE - write_offset - length));
        if (length + write_offset == BUFFER_SIZE) {
            write_it++;
            write_index++;
        }
    } else {
        uint32_t mask = 0xffffffff;
        mask >>= write_offset;
        uint32_t rvalue = value >> (length - (BUFFER_SIZE - write_offset));
        rvalue &= mask;
        *write_it |= rvalue;
        write_it++;
        write_index++;
        uint32_t lvalue = value << (2 * BUFFER_SIZE - length - write_offset);
        *write_it |= lvalue;
    }
    write_offset = (write_offset + length) % BUFFER_SIZE;
}

uint32_t BitBuffer::read(uint8_t length)
{
    uint32_t value = 0xffffffff;
    if (length + read_offset <= BUFFER_SIZE) {
        value >>= (BUFFER_SIZE - length);
        value <<= (BUFFER_SIZE - read_offset - length);
        value &= *read_it;
        value >>= (BUFFER_SIZE - read_offset - length);
        if (length + read_offset == BUFFER_SIZE) {
            read_it++;
            read_index++;
        }
    } else {
        uint32_t rmask = 0xffffffff;
        rmask >>= (read_offset);
        rmask &= *read_it;
        rmask <<= (length + read_offset - BUFFER_SIZE);
        read_it++;
        read_index++;
        value = rmask;
        uint32_t lmask = 0xffffffff;
        lmask <<= (2 * BUFFER_SIZE - length - read_offset);
        lmask &= *read_it;
        lmask >>= (2 * BUFFER_SIZE - length - read_offset);
        value = rmask | lmask;
    }
    read_offset = (read_offset + length) % BUFFER_SIZE;
    return value;
}

void BitBuffer::print(void)
{
    std::vector<uint32_t>::iterator it = buffer.begin();
    printf("Size: %d bytes\n", size());
    int i = 0;
    for (; i < write_index - 1; i++) {
        uint32_t b = *it;
        uint32_t mask = 1 << (BUFFER_SIZE - 1);
        printf("%2d: ", i);
        for (int j = 0; j < BUFFER_SIZE; ++j) {
            printf("%u", b&mask ? 1 : 0);
            b <<= 1;
        }
        ++it;
    }
    if (write_offset != 0) {
        printf("%2d: ", i);
        uint32_t b = *it;
        uint32_t mask = 1 << (BUFFER_SIZE - 1);
        for (int j = 0; j < write_offset; ++j) {
            printf("%u", b&mask ? 1 : 0);
            b <<= 1;
        }
    }
}

int BitBuffer::size(void)
{
    int remainder = write_offset / 8 + (write_offset % 8 > 0 ? 1 : 0);
    return write_index * BUFFER_SIZE / 8 + remainder;
}

int BitBuffer::read_is_end(void)
{
    return read_offset >=write_offset && write_it == read_it;
}

void BitBuffer::data_expand(void)
{
    // Double the buffer length if near capacity
    //if (index >= 0.9 * size) {
    //    uint32_t *new_buffer = (uint32_t *) realloc(buffer, size * 2 * BUFFER_SIZE / 8);
    //    buffer = new_buffer;
    //    write_it = new_buffer +=write_index;
    //    read_it = new_buffer += read_index;
    //    //memset((void * ) write_it + 1, 0, size * BUFFER_SIZE / 16);
    //    alloc_size *= 2;
   // }
}

int main ()
{
    BitBuffer b;
    b.write(10, 4);
    uint32_t t = b.read(4);
    b.write(12,4);
    b.write(200,32);
    printf("%d\n", t);
    t = b.read(4);
    printf("%d\n", t);
    t = b.read(32);
    printf("%d\n", t);
}