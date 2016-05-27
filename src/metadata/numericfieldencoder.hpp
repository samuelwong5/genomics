#ifndef NFE_HPP
#define NFE_HPP

#include "metadatafieldencoder.hpp"

#include <set>
#include <tuple>
#include <vector>

class NumericFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t width;
        bool increment; 
        uint32_t prev;
        
    public:
        NumericFieldEncoder(const std::shared_ptr<BitBuffer>&, uint32_t, bool);
        void decode_metadata(void);
        void encode_metadata(void);
        void decode(std::stringstream&);
        void encode(std::string);
};

#endif