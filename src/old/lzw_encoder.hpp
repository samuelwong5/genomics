#ifndef LZW_ENCODER_HPP
#define LZW_ENCODER_HPP

#include <cmath>
#include <string>
#include <vector>

#include "encoder.hpp"
#include "bitbuffer.hpp"

class LZWEncoder : public Encoder
{
  public:
    LZWEncoder(std::string const&);
    std::shared_ptr<BitBuffer> encode(std::string const&);
    std::string decode(std::shared_ptr<BitBuffer>);
  private:
    std::vector<std::string> dict;
    uint8_t enc_size;
    uint32_t max_entry;
    int alphabet_size;
    void reset(void);
};

#endif