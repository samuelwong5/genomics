#ifndef MFE_HPP
#define MFE_HPP

#include <cstdio>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "../bitbuffer/bitbuffer.hpp"
#include "encodeutil.hpp"

class MetadataFieldEncoder {
    protected:
      const std::shared_ptr<BitBuffer> buffer;                           // Pointer to BitBuffer
      std::shared_ptr<std::vector<bb_entry_t> > encoded;
      
    public:
      MetadataFieldEncoder(const std::shared_ptr<BitBuffer>&);           // Constructor
      ~MetadataFieldEncoder() { }
      virtual void decode_metadata(void) = 0;                            // Decode metadata from BitBuffer
      virtual void encode_metadata(void) = 0;                            // Encode metadata from BitBuffer
      virtual char* decode(char *) = 0;                            // Decode next entry for a field
      virtual bool encode(std::string) = 0;                              // Encode next entry for a field
      virtual uint32_t get_width(void) = 0;                              // Get the width of each field entry   
      std::shared_ptr<BitBuffer> get_buffer(void) { return buffer; }     // DEBUG ONLY - get BitBuffer member
      
      // Helpers for multithreading
      void set_encoded(std::shared_ptr<std::vector<bb_entry_t> > enc) { encoded = enc; }
      virtual MetadataFieldEncoder* clone(void) = 0;
};

#endif
