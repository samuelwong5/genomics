#include "autoincrementingfieldencoder.hpp"

AutoIncrementingFieldEncoder::AutoIncrementingFieldEncoder(std::shared_ptr<BitBuffer> buffer, uint32_t init) 
    : MetadataFieldEncoder(buffer) 
{
    current = init;
}

void 
AutoIncrementingFieldEncoder::encode(std::string s)
{
    
}

void
AutoIncrementingFieldEncoder::decode(std::stringstream& ss)
{
    ss << current++;
}