#ifndef ANFE_HPP
#define ANFE_HPP

#include <cstring>
#include <set>
#include <tuple>
#include <vector>

#include "metadatafieldencoder.hpp"
#include "encodeutil.hpp"


class AlphanumericFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t width;
        uint32_t mappings;
        bool enable_map;       
        std::vector<std::string> map;
        
    public:
        AlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&);
        AlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&, uint32_t, bool, std::set<std::string>);
        AlphanumericFieldEncoder(const AlphanumericFieldEncoder& afe) : MetadataFieldEncoder(afe.buffer)
        {
            width = afe.width;
            mappings = afe.mappings;
            enable_map = afe.enable_map;
            map = afe.map;
        }
        MetadataFieldEncoder* clone(void) { return new AlphanumericFieldEncoder(*this); }
        
        void decode_metadata(void);
        void encode_metadata(void);
        bool encode(std::string);
        char* decode(char *);
        uint32_t get_width(void) { return width; }
};

#endif
