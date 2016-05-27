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
    // Field type = 1
    width = buffer->read(14);
}

void 
NumericFieldEncoder::encode_metadata(void)
{
    // Field type = 10
    buffer->write(8, 4);
    
    // Value
    buffer->write(width, 12);
}

void 
NumericFieldEncoder::encode(std::string s)
{
    int val = atoi(s.c_str());
    if (increment)
    {
        buffer->write(val - prev, width);
        prev = val;
    }
    else
    {
        buffer->write(val, width);
    }
}

void
NumericFieldEncoder::decode(std::ostream& ss)
{
    int delta = (int) buffer->read(width);
    if (increment)
    {  
       int curr = delta + prev;
       prev = curr;
       ss << curr; 
    }
    else
    {
        ss << delta;
    }
}