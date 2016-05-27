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
    expand();
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
    for (; i < write_index; i++) {
        uint32_t b = *it;
        uint32_t mask = 1 << (BUFFER_SIZE - 1);
        printf("%2d: ", i);
        for (int j = 0; j < BUFFER_SIZE; ++j) {
            printf("%u", b&mask ? 1 : 0);
            b <<= 1;
        }
        ++it;
        printf("\n");
    }
    if (write_offset != 0) {
        printf("%2d: ", i);
        uint32_t b = *it;
        uint32_t mask = 1 << (BUFFER_SIZE - 1);
        for (int j = 0; j < write_offset; ++j) {
            printf("%u", b&mask ? 1 : 0);
            b <<= 1;
        }     
        printf("\n");
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

void BitBuffer::expand(void)
{
    if (write_index + 1 >= alloc_size) {
        alloc_size = alloc_size * 2;
        try {
            buffer.resize(alloc_size, 0);           
        } catch (const std::bad_alloc&) {
            exit(-1);
        }
        write_it = buffer.begin();
        std::advance(write_it, write_index);
        read_it = buffer.begin();
        std::advance(read_it, read_index);
   }
}

void 
BitBuffer::write_to_file(std::string filename)
{
    std::ofstream fout(filename, std::ios::out | std::ios::binary);
    fout.write((char*)&buffer[0], (write_index + 1) * 4);    
}

void 
BitBuffer::read_from_file(std::string filename)
{
    std::ifstream fs(filename, std::ifstream::ate | std::ifstream::binary);
    std::streamsize size = fs.tellg();
    //std::cout << "Filesize: " << size << "\n";
    buffer.resize(size / 4);
    write_it = buffer.begin();
    read_it = buffer.begin();
    //std::cout << "Buffer size: " << buffer.size() << "\n";
    fs.seekg( 0, std::ios::beg );
    write_index = size / 4;
    fs.read((char*)buffer.data(), size); 
}