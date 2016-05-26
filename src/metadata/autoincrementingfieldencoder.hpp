#ifndef AIFE_HPP
#define AIFE_HPP

#include "metadatafieldencoder.hpp"

class AutoIncrementingFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t current;
    public:
        AutoIncrementingFieldEncoder(std::shared_ptr<BitBuffer>, uint32_t);
        void encode(std::string);
        void decode(std::stringstream&);
};

#endif