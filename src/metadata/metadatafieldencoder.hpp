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
      const std::shared_ptr<BitBuffer> buffer;
    public:
      MetadataFieldEncoder(const std::shared_ptr<BitBuffer>&);
      virtual void decode_metadata(void) = 0;
      virtual void encode_metadata(void) = 0;
      virtual void decode(std::stringstream&) = 0;
      virtual void encode(std::string) = 0;
      std::shared_ptr<BitBuffer> get_buffer(void) { return buffer; }
};

#endif
