#ifndef BITBUFFER_HPP
#define BITBUFFER_HPP

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
 * Utility functions to writing individual bits into a buffer to
 * increase compression rates. Uses a uint32_t buffer so a maximum
 * of 32bits can be written or read at the same time. 
 */

struct bb_entry_t {
    uint32_t value;
    uint8_t length;    
};

class BitBuffer {
  private:
    std::vector<uint32_t> buffer;               // Buffer where the bits are stored
    
    std::vector<uint32_t>::iterator write_it;   // Iterator for next write
    uint32_t write_offset;                      // Bit offset of write pointer
    uint32_t write_index;                       // Size of written memory
    
    std::vector<uint32_t>::iterator read_it;    // Iterator for next read
    uint32_t read_offset;                       // Bit offset of read pointer
    uint32_t read_index;                        // Size of read memory
    
    uint32_t alloc_size;                        // Size of allocated memory
    void expand(void);                          // Resize vector buffer if fully used
    
  public:
    BitBuffer();
    ~BitBuffer();
    void init(void);                            // Reset members
    
    void write(uint32_t, uint8_t);              // Write a value into the buffer
    uint32_t read(uint8_t);                     // Read a value from the buffer
    
    void write_to_file(std::string);            // Write buffer to file
    void read_from_file(std::string);           // Read buffer from file
    
    bool read_is_end(void);                     // Check if read_it at end of buffer
    void read_seek(uint32_t);                   // Seek read pointer
    void read_pad(void);                        // Seek read pointer to start of next uint32_t
    void write_pad(void);                       // Pad write pointer to start of next uint32_t
    
    int size(void);                             // Returns size of the buffer in bytes
    void print(void);                           // Print the buffer in binary
    
    void read_pad_back(void){ read_offset = 0; }// Seek read pointer to start of current uint32_t
    
    /* DEBUG UTILS */
    void print_read_ptr(void) 
    { 
        std::cout << read_index << " [" << read_offset << "] (total: " << buffer.size() << ")\n"; 
    }
    
    void print_section(int size)
    {
        for (int i = 0; i < size; i++)
            std::cout << (read_index + i) << ": " << buffer[read_index + i] << std::endl;
    }
};

#endif
