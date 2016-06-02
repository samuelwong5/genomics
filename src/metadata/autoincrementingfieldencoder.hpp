#ifndef AIFE_HPP
#define AIFE_HPP

#include "metadatafieldencoder.hpp"


class AutoIncrementingFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t current;
        
    public:
        AutoIncrementingFieldEncoder(const std::shared_ptr<BitBuffer>&);
        AutoIncrementingFieldEncoder(const std::shared_ptr<BitBuffer>&, uint32_t);
        AutoIncrementingFieldEncoder(const AutoIncrementingFieldEncoder& aife) : MetadataFieldEncoder(aife.buffer)
        {
            //current = aife.current;
        }
        MetadataFieldEncoder* clone(void) { return new AutoIncrementingFieldEncoder(*this); }
        
        void decode_metadata(void);
        void encode_metadata(void);
        void decode(std::ostream&);
        void encode(std::string);
        void set_current(uint32_t c) { current = c; } // For multithreading
        uint32_t get_width(void) { return 0; }
};

#endif