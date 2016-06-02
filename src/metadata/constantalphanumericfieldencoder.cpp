#include "constantalphanumericfieldencoder.hpp"


ConstantAlphanumericFieldEncoder::ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer)
    : MetadataFieldEncoder(buffer)
{
      
}


ConstantAlphanumericFieldEncoder::ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, std::string v = std::string()) 
    : MetadataFieldEncoder(buffer), value(v)
{

}


void 
ConstantAlphanumericFieldEncoder::decode_metadata(void)
{
    int width = buffer->read(14);
    while (width --> 0)
        value.append(1, buffer->read(8));
}


void 
ConstantAlphanumericFieldEncoder::encode_metadata(void)
{
    // Field type = 00
    buffer->write(0, 4);
    
    // Value
    buffer->write(value.length(), 12);
    for (auto it = value.begin(); it != value.end(); ++it)
        buffer->write(*it, 8);
}


void 
ConstantAlphanumericFieldEncoder::encode(std::string s)
{

}


void
ConstantAlphanumericFieldEncoder::decode(std::ostream& ss)
{
    ss << value;
}