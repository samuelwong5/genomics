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
        bool enable_map;       // 1 if use int->string mapping, 0 otherwise
        std::vector<std::string> map;
        
    public:
        AlphanumericFieldEncoder(std::shared_ptr<BitBuffer>, uint32_t, bool, std::set<std::string>);
        void encode(std::string);
        void decode(std::stringstream&);
};

#endif