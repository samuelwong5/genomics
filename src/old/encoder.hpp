#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <memory>
#include <string>
#include "bitbuffer.hpp"

class Encoder
{
  public:
    virtual std::shared_ptr<BitBuffer> encode(std::string const&) = 0;
    virtual std::string decode(std::shared_ptr<BitBuffer>) = 0;
};

#endif