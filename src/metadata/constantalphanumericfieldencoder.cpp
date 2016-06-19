#include "constantalphanumericfieldencoder.hpp"


ConstantAlphanumericFieldEncoder::ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer)
    : MetadataFieldEncoder(buffer)
{
      
}


ConstantAlphanumericFieldEncoder::ConstantAlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, std::string v = std::string()) 
    : MetadataFieldEncoder(buffer), value(v)
{

}


ConstantAlphanumericFieldEncoder::ConstantAlphanumericFieldEncoder(const ConstantAlphanumericFieldEncoder& cafe) : MetadataFieldEncoder(cafe.buffer)
{
    value = std::string(cafe.value);
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


bool 
ConstantAlphanumericFieldEncoder::encode(std::string s)
{
    return s == value;
}


char *
ConstantAlphanumericFieldEncoder::decode(char * md)
{
    for (uint32_t i = 0; i < value.length(); i++)
        *(md++) = value[i];
    return md;
}


uint32_t 
ConstantAlphanumericFieldEncoder::get_width(void) 
{ 
    return 0; 
}
