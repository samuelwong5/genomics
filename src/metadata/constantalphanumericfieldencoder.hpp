#ifndef CANFE_HPP
#define CANFE_HPP

#include "metadatafieldencoder.hpp"

class ConstantAlphanumericFieldEncoder : public MetadataFieldEncoder {
    private:
        std::string value;
        
    public:
        ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&);
        ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&, std::string);
        ConstantAlphanumericFieldEncoder(const ConstantAlphanumericFieldEncoder&);
        ~ConstantAlphanumericFieldEncoder() { }
        MetadataFieldEncoder* clone(void) { return new ConstantAlphanumericFieldEncoder(*this); }
        
        void decode_metadata(void);
        void encode_metadata(void);
        char* decode(char *);
        bool encode(std::string);
        uint32_t get_width(void);
};

#endif
