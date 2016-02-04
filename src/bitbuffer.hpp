#ifndef BITBUFFER_HPP
#define BITBUFFER_HPP

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

/*
 * Utility functions to writing individual bits into a buffer to
 * increase compression rates. Uses a uint32_t buffer so a maximum
 * of 32bits can be written or read at the same time. 
 */

class BitBuffer {
  private:
    std::vector<uint32_t> buffer;               // Buffer where the bits are stored
    std::vector<uint32_t>::iterator write_it;  // Iterator for next write
    uint8_t write_offset;                       // Bit offset of write pointer
    uint8_t write_index;                        // Size of written memory
    std::vector<uint32_t>::iterator read_it;   // Iterator for next read
    uint8_t read_offset;                        // Bit offset of read pointer
    uint8_t read_index;                         // Size of read memory
    uint8_t alloc_size;                         // Size of allocated memory
    void data_expand(void);
  public:
    ~BitBuffer();
    BitBuffer();
    void write(uint32_t, uint8_t);
    uint32_t read(uint8_t);
    void print(void);
    int read_is_end(void);
    int size(void);
};

#endif
