#ifndef CANFE_HPP
#define CANFE_HPP

#include "metadatafieldencoder.hpp"

class ConstantAlphanumericFieldEncoder : public MetadataFieldEncoder {
    private:
        std::string value;
    public:
        ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&);
        ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&, std::string);
        void decode_metadata(void);
        void encode_metadata(void);
        void decode(std::ostream&);
        void encode(std::string);
};

#endif