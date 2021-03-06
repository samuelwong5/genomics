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
        }
        mappings = w;
        width = EncodeUtil::ceil_log(mappings, 2);
    }
    else
    {
        width = w;
    }
}


AlphanumericFieldEncoder::AlphanumericFieldEncoder(const AlphanumericFieldEncoder& afe) : MetadataFieldEncoder(afe.buffer)
{
    width = afe.width;
    mappings = afe.mappings;
    enable_map = afe.enable_map;
    map = afe.map;
}
        
        
void 
AlphanumericFieldEncoder::decode_metadata(void)
{
    int k = buffer->read(2);
    if (k & 0x2)
    {
        enable_map = true;
    } else {
        enable_map = false;
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
        int pad = width - s.length() * 8;
        
        // If current max allowed width is surpassed, 
        // return false to reanalyze metadata
        if (pad < 0) {
            return false;
        }
        for (std::string::iterator it = s.begin(); it != s.end(); ++it)
            EncodeUtil::bb_entry(*it, 8, encoded);
        while (pad >= 32) 
        {
            EncodeUtil::bb_entry(0, 32, encoded);           
            pad -= 32;
        }
        EncodeUtil::bb_entry(0, pad, encoded);
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
    else 
    {
        int num_chars = width / 8;
        while (num_chars--) 
        {
            char c = buffer->read(8);
            if (!c) { break; }
            *(md++) = c;          
        }
        
        // Offset bitbuffer read pointer
        while (num_chars-- > 0) 
        {
            buffer->read(8);
        }
    }
    return md;
}


uint32_t 
AlphanumericFieldEncoder::get_width(void)
{ 
    return width; 
}
        
        