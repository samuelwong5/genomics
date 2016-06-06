#include "numericfieldencoder.hpp"


NumericFieldEncoder::NumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer)
    : MetadataFieldEncoder(buffer)
{
        
}


NumericFieldEncoder::NumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, uint32_t w, bool i) 
    : MetadataFieldEncoder(buffer), width(w), increment(i), prev(0)
{
    
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
    //EncodeUtils::bb_entry(8, 4, encoded);
    
    // Width
    buffer->write(width, 12);
    //EncodeUtils::bb_entry(width, 12, encoded);    
}


void 
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
        EncodeUtil::bb_entry(val, width, encoded);
    }
}


char *
NumericFieldEncoder::decode(char* md)
{
    int delta = (int) buffer->read(width);
    if (increment)
    {  
       int curr = delta + prev;
       prev = curr;
       return md + sprintf(md, "%d", curr);
    }
    else
    {
       return md + sprintf(md, "%d", delta);
    }
}

