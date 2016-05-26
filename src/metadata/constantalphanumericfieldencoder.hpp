#ifndef CANFE_HPP
#define CANFE_HPP

#include "metadatafieldencoder.hpp"

class ConstantAlphanumericFieldEncoder : public MetadataFieldEncoder {
    private:
        std::string value;
    public:
        ConstantAlphanumericFieldEncoder(std::shared_ptr<BitBuffer>, std::string);
        void encode(std::string);
        void decode(std::stringstream&);
};

#endif