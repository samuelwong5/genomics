#ifndef ANFE_HPP
#define ANFE_HPP

#include "metadatafieldencoder.hpp"

#include <cstring>
#include <set>
#include <tuple>
#include <vector>

class AlphanumericFieldEncoder : public MetadataFieldEncoder {
    private:
        uint32_t width;
        uint32_t mappings;
        bool enable_map;       // 1 if use int->string mapping, 0 otherwise
        std::vector<std::string> map;
        
    public:
        AlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&);
        AlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>&, uint32_t, bool, std::set<std::string>);
        void decode_metadata(void);
        void encode_metadata(void);
        void encode(std::string);
        void decode(std::ostream&);
        uint32_t get_width(void) { return width; }
};

#endif