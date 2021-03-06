#include "numericfieldencoder.hpp"


NumericFieldEncoder::NumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer)
    : MetadataFieldEncoder(buffer), increment(false), width(0), prev(0)
{
        
}


NumericFieldEncoder::NumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, uint32_t w, bool i) 
    : MetadataFieldEncoder(buffer), increment(i), width(w), prev(0)
{
    
}


NumericFieldEncoder::NumericFieldEncoder(const NumericFieldEncoder& nfe) : MetadataFieldEncoder(nfe.buffer)
{
    width = nfe.width;
    increment = nfe.increment;
    prev = nfe.prev;
}
        
        
void 
NumericFieldEncoder::decode_metadata(void)
{
    increment = false;
    width = buffer->read(14);
}


void 
NumericFieldEncoder::encode_metadata(void)
{
    // Field type = 10
    buffer->write(8, 4);
    
    // Width
    buffer->write(width, 12);  
}


bool 
NumericFieldEncoder::encode(std::string s)
{
    int val = atoi(s.c_str());
    if (increment)
    {
        EncodeUtil::bb_entry(val - prev, width, encoded);
        prev = val;
    }
    else
    {
        if (EncodeUtil::ceil_log(val, 2) > width)
            return false;
        EncodeUtil::bb_entry(val, width, encoded);
    }
    return true;
}


char *
NumericFieldEncoder::decode(char* md)
{
    uint32_t delta = buffer->read(width);
    if (increment)
    {  
       int curr = delta + prev;
       prev = curr;
       return md + sprintf(md, "%d", curr);
    }
    else
    {
       return md + sprintf(md, "%u", delta);
    }
}


uint32_t 
NumericFieldEncoder::get_width(void)
{
    return width;
}    
    