#ifndef MFE_HPP
#define MFE_HPP

#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "bitbuffer.hpp"


class MetadataFieldEncoder {
    protected:
      const std::shared_ptr<BitBuffer> buffer;                           // Pointer to BitBuffer
      
    public:
      MetadataFieldEncoder(const std::shared_ptr<BitBuffer>&);           // Constructor
      virtual void decode_metadata(void) = 0;                            // Decode metadata from BitBuffer
      virtual void encode_metadata(void) = 0;                            // Encode metadata from BitBuffer
      virtual void decode(std::ostream&) = 0;                            // Decode next entry for a field
      virtual void encode(std::string) = 0;                              // Encode next entry for a field
      std::shared_ptr<BitBuffer> get_buffer(void) { return buffer; }     // DEBUG ONLY - get BitBuffer member
      virtual uint32_t get_width(void) = 0;                              // Get the width of each field entry
};

#endif
