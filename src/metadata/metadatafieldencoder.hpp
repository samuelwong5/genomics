#ifndef MFE_HPP
#define MFE_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "bitbuffer.hpp"

class MetadataFieldEncoder {
    protected:
      std::shared_ptr<BitBuffer> buffer;
    public:
      MetadataFieldEncoder(std::shared_ptr<BitBuffer>);
      virtual void decode(std::stringstream& ss) { };
      virtual void encode(std::string s) { };
};

#endif
