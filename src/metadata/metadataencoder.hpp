#ifndef MDE_HPP
#define MDE_HPP

#include <cstring>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>

#include "../sequence/reads.hpp"

#include "metadatafieldencoder.hpp"
#include "alphanumericfieldencoder.hpp"
#include "autoincrementingfieldencoder.hpp"
#include "constantalphanumericfieldencoder.hpp"
#include "numericfieldencoder.hpp"
#include "encodeutil.hpp"

enum MetadataFieldType {
    CONSTANT_ALPHANUMERIC = 0,
    ALPHANUMERIC = 1,
    NUMERIC = 2,
    AUTOINCREMENTING = 3
};

struct MetadataAnalysis {
    MetadataFieldType type;
    uint32_t width;
    std::string val;
};


class MetaDataEncoder {
private:
    const std::shared_ptr<BitBuffer> b;
    uint8_t num_fields;
    uint8_t num_sep;
    std::vector<char> sep;
    std::vector<MetadataFieldEncoder*> fields;
    uint32_t total_entries = 0; // For decoding autoincrementfieldenc starting point
    static const uint32_t MAGIC_NUMBER = 0x18318525;
    
public:
    void metadata_separators(std::string metadata);
    static void split(std::string& str, std::vector<std::string>& parts);
    void metadata_analyze(std::vector<read_t>&, uint32_t);
    void encode_separators(void);
    void decode_separators(void);
    MetaDataEncoder() : b(std::shared_ptr<BitBuffer>(new BitBuffer)) {  }
    MetaDataEncoder(char *);
    ~MetaDataEncoder() 
    {   // Cleanup
        for (auto it = fields.begin(); it != fields.end(); it++)
        {
            delete *it;
        }
    }
    void decode_fields(void);
    void decode_entry(read_t&);
    bool metadata_decompress(std::vector<read_t>&, char *);
    void metadata_compress(std::vector<read_t>&, char *);
};

#endif
