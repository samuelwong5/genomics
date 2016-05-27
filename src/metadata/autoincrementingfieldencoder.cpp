#include "autoincrementingfieldencoder.hpp"

AutoIncrementingFieldEncoder::AutoIncrementingFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, uint32_t init) 
    : MetadataFieldEncoder(buffer), current(init)
{

}

void 
AutoIncrementingFieldEncoder::decode_metadata(void)
{
    // Field type = 1
    current = buffer->read(30);
}

void 
AutoIncrementingFieldEncoder::encode_metadata(void)
{
    // Field type = 11 Reserved flag = 00
    buffer->write(12, 4);
    
    // Initual value
    buffer->write(current, 28);
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