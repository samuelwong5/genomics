#ifndef NFE_HPP
#define NFE_HPP

#include <set>
#include <tuple>
#include <vector>

#include "metadatafieldencoder.hpp"


class NumericFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t width;
        bool increment; 
        uint32_t prev;
        
    public:
        NumericFieldEncoder(const std::shared_ptr<BitBuffer>&);
        NumericFieldEncoder(const std::shared_ptr<BitBuffer>&, uint32_t, bool);
        NumericFieldEncoder(const NumericFieldEncoder& nfe) : MetadataFieldEncoder(nfe.buffer)
        {
            width = nfe.width;
            increment = nfe.increment;
            prev = nfe.prev;
        }
        MetadataFieldEncoder* clone(void) { return new NumericFieldEncoder(*this); }
        
        void decode_metadata(void);
        void encode_metadata(void);
        char* decode(char *);
        void encode(std::string);
        uint32_t get_width(void) { return width; }
};

#endif