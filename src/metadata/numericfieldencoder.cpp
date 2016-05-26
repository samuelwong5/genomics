#include "numericfieldencoder.hpp"

NumericFieldEncoder::NumericFieldEncoder(std::shared_ptr<BitBuffer> buffer, uint32_t w, bool i) 
    : MetadataFieldEncoder(buffer), width(w), increment(i), prev(0)
{
    
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
NumericFieldEncoder::decode(std::stringstream& ss)
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