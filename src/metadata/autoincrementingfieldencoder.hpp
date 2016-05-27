#ifndef AIFE_HPP
#define AIFE_HPP

#include "metadatafieldencoder.hpp"

class AutoIncrementingFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t current;
    public:
        AutoIncrementingFieldEncoder(const std::shared_ptr<BitBuffer>&, uint32_t);
        void decode_metadata(void);
        void encode_metadata(void);
        void decode(std::stringstream&);
        void encode(std::string);
};

#endif