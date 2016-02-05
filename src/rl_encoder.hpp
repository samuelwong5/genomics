#ifndef RL_ENCODER_HPP
#define RL_ENCODER_HPP

#include <cmath>
#include "encoder.hpp"

class RlEncoder : public Encoder
{
    public:
      RlEncoder(std::string const&, uint8_t);
      std::shared_ptr<BitBuffer> encode(std::string const&);
      std::string decode(std::shared_ptr<BitBuffer>);
    private:
      uint8_t offset;
      uint8_t enc_char_length;
      uint8_t enc_count_length;
};

#endif
