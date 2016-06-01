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


class MetaDataEncoder {
private:
    const std::shared_ptr<BitBuffer> b;
    uint8_t num_fields;
    std::vector<char> sep;
    std::vector<MetadataFieldEncoder*> fields;
    
public:
    void metadata_separators(std::string metadata);
    static void split(std::string& str, std::vector<std::string>& parts);
    void metadata_analyze(std::vector<read_t>&, int);
    void encode_separators(void);
    void decode_separators(void);
    MetaDataEncoder() : b(std::shared_ptr<BitBuffer>(new BitBuffer)) { std::cout << "MDEBB.get()" << b.get() << std::endl; }
    void metadata_compress(std::vector<read_t>&, char *);
};

#endif