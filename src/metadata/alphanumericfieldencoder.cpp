#include "alphanumericfieldencoder.hpp"

AlphanumericFieldEncoder::AlphanumericFieldEncoder(std::shared_ptr<BitBuffer> buffer, uint32_t w, bool em, std::set<std::string> values)
    : MetadataFieldEncoder(buffer), width(w), enable_map(em)
{
    if (enable_map)
    {
        for (std::set<std::string>::iterator it = values.begin(); it != values.end(); ++it)
        {
            map.push_back(*it);
        }
    }
}

void 
AlphanumericFieldEncoder::encode(std::string s)
{
    if (enable_map)
    {
        // Find corresponding key in map
        int len = s.length();
        uint32_t key = 0;
        for (std::vector<std::string>::iterator it = map.begin(); it != map.end(); ++it)
        {
            if (!strncmp(s.c_str(), it->c_str(), len)) 
            {
                buffer->write(key, width);
                break;
            }
            key++;
        }
    } 
    else
    {
        // Write each character to bitbuffer using 8 bits
        int pad = width - s.length() * 8;
        for (std::string::iterator it = s.begin(); it != s.end(); ++it)
            buffer->write(*it, 8);
        while (pad >= 32) 
        {
            buffer->write(0, 32);
            pad -= 32;
        }
        buffer->write(0, pad);
    }
}

void
AlphanumericFieldEncoder::decode(std::stringstream& ss)
{
    if (enable_map)
    {
        uint32_t key = buffer->read(width);
        ss << map[key];
    } 
    else
    {
        int size = width;
        while (size > 0) 
        {
            char c = buffer->read(8);
            size -= 8;
            if (!c) { break; }
            ss << c;            
        }
        
        // Offset bitbuffer read pointer
        while (size >= 32) 
        {
            buffer->read(32);
            size -= 32;
        }
        buffer->read(size);
    }
}