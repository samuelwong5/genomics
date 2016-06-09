#include "alphanumericfieldencoder.hpp"


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
            //std::cout << "      - " << map.size() - 1 << ": " << *it << std::endl;
        }
        mappings = w;
        width = EncodeUtil::ceil_log(mappings, 2);
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
        width = EncodeUtil::ceil_log(mappings, 2);
        for (uint32_t i = 0; i < mappings; i++)
        {
            std::string s("");
            int curr;
            while ((curr = buffer->read(8)))
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
    // Field type = 01
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


bool
AlphanumericFieldEncoder::encode(std::string s)
{
    if (enable_map)
    {
        // Find corresponding key in map
        uint32_t key = 0;
        for (std::vector<std::string>::iterator it = map.begin(); it != map.end(); ++it)
        {
            if (s == *it) 
            {
                EncodeUtil::bb_entry(key, (uint8_t) width, encoded);
                break;
            }
            key++;
        }
        if (key == map.size()) { return false; }
    } 
    else
    {
        // Write each character to bitbuffer using 8 bits
        //int pad = width - s.length() * 8;
        //for (std::string::iterator it = s.begin(); it != s.end(); ++it)
        //    buffer->write(*it, 8);
        //while (pad >= 32) 
       // {
       //     buffer->write(0, 32);
       //     pad -= 32;
       // }
       // buffer->write(0, pad);
    }
    return true;
}


char *
AlphanumericFieldEncoder::decode(char *md)
{
    if (enable_map)
    {
        uint32_t key = buffer->read(width);
        std::string value = map[key];
        for (uint32_t i = 0; i < value.length(); i++)
            *(md++) = value[i];

    } 
    else // TODO: Test integrity of non-mapped values
    {
        /*int size = width;
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
        buffer->read(size);*/
    }
    return md;
}
