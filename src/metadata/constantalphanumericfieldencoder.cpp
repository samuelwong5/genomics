#include "constantalphanumericfieldencoder.hpp"

ConstantAlphanumericFieldEncoder::ConstantAlphanumericFieldEncoder(std::shared_ptr<BitBuffer> buffer, std::string v) 
    : MetadataFieldEncoder(buffer) 
{
    value = v;
}

void 
ConstantAlphanumericFieldEncoder::encode(std::string s)
{
    
}

void
ConstantAlphanumericFieldEncoder::decode(std::stringstream& ss)
{
    ss << value;
}