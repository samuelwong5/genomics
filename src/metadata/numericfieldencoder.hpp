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
        NumericFieldEncoder(std::shared_ptr<BitBuffer>, uint32_t, bool);
        void encode(std::string);
        void decode(std::stringstream&);
};

#endif