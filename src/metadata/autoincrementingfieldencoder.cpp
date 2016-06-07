#include "autoincrementingfieldencoder.hpp"


AutoIncrementingFieldEncoder::AutoIncrementingFieldEncoder(const std::shared_ptr<BitBuffer>& buffer)
    : MetadataFieldEncoder(buffer)
{
        
}


AutoIncrementingFieldEncoder::AutoIncrementingFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, uint32_t init) 
    : MetadataFieldEncoder(buffer), current(init)
{

}


void 
AutoIncrementingFieldEncoder::decode_metadata(void)
{
    current += buffer->read(30);
}



void 
AutoIncrementingFieldEncoder::encode_metadata(void)
{
    // Field type = 11 Reserved flag = 00
    buffer->write(12, 4);
    
    // Initial value
    buffer->write(current, 28);
}


bool
AutoIncrementingFieldEncoder::encode(std::string s)
{
    return true;
}


char *
AutoIncrementingFieldEncoder::decode(char* md)
{
    return md + sprintf(md, "%d", current++);
}
