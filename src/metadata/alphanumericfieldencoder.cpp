#include "alphanumericfieldencoder.hpp"

int 
ceillog(int max, int base)
{
    int w = 1;
    int curr = base;
    while (curr <= max)
    {
        w++;
        curr *= base;
    }
    return w;
}

AlphanumericFieldEncoder::AlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer)
    : MetadataFieldEncoder(buffer)
{
        
}

AlphanumericFieldEncoder::AlphanumericFieldEncoder(const std::shared_ptr<BitBuffer>& buffer, uint32_t w = 0, bool em = false, std::set<std::string> values = {})
    : MetadataFieldEncoder(buffer), enable_map(em)
{
    if (enable_map)
    {
        for (std::set<std::string>::iterator it = values.begin(); it != values.end(); ++it)
        {
            map.push_back(*it);
        }
        mappings = w;
        width = ceillog(mappings, 2);
    }
    else
    {
        width = w;
    }
}

void 
AlphanumericFieldEncoder::decode_metadata(void)
{
    int k = buffer->read(2);
    if (k & 2)
    {
        enable_map = true;
    }
    
       
    // Read mapped values
    if (enable_map)
    {
        mappings = buffer->read(12);
        width = ceillog(mappings, 2);
        for (uint32_t i = 0; i < mappings; i++)
        {
            std::string s("");
            int curr;
            while (curr = buffer->read(8))
            {
                s.append(1, curr);
            }
            map.push_back(s);
        }
    }
    else
    {
        width = buffer->read(12);
    }
}

void 
AlphanumericFieldEncoder::encode_metadata(void)
{
    // Field type = 1
    buffer->write(1,2);

    // Enable map flag
    if (enable_map)
    {
        buffer->write(2,2);
        buffer->write(mappings, 12);
        for (std::vector<std::string>::iterator it = map.begin(); it != map.end(); ++it)
        {
            for (std::string::iterator mit = it->begin(); mit != it->end(); ++mit)
                buffer->write(*mit, 8);
            buffer->write(0, 8);
        }
    }
    else
    {
        buffer->write(0,2);
        buffer->write(width, 12);
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
AlphanumericFieldEncoder::decode(std::ostream& ss)
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