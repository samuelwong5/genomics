#include <fstream>
#include <vector>
#include <iostream>
#include <set>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "metadatafieldencoder.hpp"
#include "autoincrementingfieldencoder.hpp"
#include "alphanumericfieldencoder.hpp"
#include "constantalphanumericfieldencoder.hpp"
#include "numericfieldencoder.hpp"
#include "bitbuffer.hpp"

int 
ceil_log(int max, int base)
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

bool 
isNumeric(const std::string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

void
metadata_separators(std::string metadata, std::vector<char>& separators)
{
    for (std::string::iterator it = metadata.begin(); it != metadata.end(); ++it)
    {
        if (*it == ' ' || *it == '.' || *it == '-' || *it == ':' || *it == '#')
            separators.push_back(*it);
    }
}

void 
split(std::string& str, std::vector<std::string>& parts) {
  size_t start, end = 0;
  static std::string delim = (" #.-:");
  while (end < str.size()) {
    start = end;
    while (start < str.size() && (delim.find(str[start]) != std::string::npos)) {
      start++;  // skip initial whitespace
    }
    end = start;
    while (end < str.size() && (delim.find(str[end]) == std::string::npos)) {
      end++; // skip to end of word
    }
    if (end-start != 0) {  // just ignore zero-length std::strings.
      parts.push_back(std::string(str, start, end-start));
    }
  }
}


void
metadata_analyze(std::ifstream &file, std::vector<char>& sep, std::vector<MetadataFieldEncoder*>& fields, int entries, const std::shared_ptr<BitBuffer> b)
{   
    std::string metadata;
    int remaining = entries - 1;
    std::getline(file, metadata);
    metadata.erase(0,1);
    metadata_separators(metadata, sep);    // List of Separators in order  
    int num_fields = sep.size() + 1;
    std::cout << "  - Analyzing Fields: [" << num_fields << "]\n";
    
    // Initialize set of values for each field to count the number of 
    // total values
    std::vector<std::set<std::string> > values;
    for (int i = 0; i < num_fields; i++)
        values.push_back(std::set<std::string>());
    {
        std::vector<std::string> v;
        split(metadata, v);
        for (int i = 0; i < num_fields; i++)
            values[i].insert(v[i]);
    }
    b->write(num_fields, 8);
    b->write(entries, 24);

    std::getline(file, metadata);
    std::getline(file, metadata);
    std::getline(file, metadata); 
    while (remaining --> 0 && std::getline(file, metadata))
    {
        metadata.erase(0,1);
        std::vector<std::string> v;
        split(metadata, v);
        for (int i = 0; i < num_fields; i++)
            values[i].insert(v[i]);
        std::getline(file, metadata);
        std::getline(file, metadata);
        std::getline(file, metadata);      
    }
    
    
    for (auto it = values.begin(); it != values.end(); it++)
    {
        std::string str("1");
        if (it->size() == 1)
        {
            std::cout << "    - Constant Alphanumeric: " << *it->begin() << std::endl;
            fields.push_back(new ConstantAlphanumericFieldEncoder(b, *it->begin()));
        }
        else if (it->size() > 0)
        {
            int max = 0;
            //int prev = -1;
            //int max_delta = 0;
            bool numeric = true;
            for (auto sit = it->begin(); sit != it->end(); sit++)
            {
                if (!numeric || !isNumeric(*sit))
                {
                    numeric = false;
                    max = ceil_log(max, 10);
                    max = max > sit->length() ? max : sit->length();
                }
                else
                {
                    int val = atoi(sit->c_str());
                    max = max > val ? max : val;
                    //prev = val;                        
                }
            }
            if (numeric)
            {
                if (max == entries)
                {
                    std::cout << "    - Auto Incrementing" << std::endl;    
                    fields.push_back(new AutoIncrementingFieldEncoder(b, 1));
                }
                else 
                {
                    std::cout << "    - Numeric  (Values: " << it->size() << " |  Max: " << max << ")\n";
                    // Non-incremental
                    fields.push_back(new NumericFieldEncoder(b, ceil_log(max + 1, 2), false));
                }
            }        
            else 
            {
                std::cout << "    - Alphanumeric  (Values: " << it->size() << ")\n";
                int bits_per_char = 8;
                if (true || it->size() * 10 < entries) // Enable mapping
                {
                    fields.push_back(new AlphanumericFieldEncoder(b, it->size(), true, *it));
                }
                else 
                {
                    fields.push_back(new AlphanumericFieldEncoder(b, max * bits_per_char, false, *it));
                }
            }
        }
    }
}

void
encode_separators(std::vector<char>& sep, const std::shared_ptr<BitBuffer>& b)
{
    static const int bits = 3;
    for (auto it = sep.begin(); it != sep.end(); it++)
    {
        switch (*it)
        {
            case ' ':
                b->write(0, bits);
                break;
            case '.':
                b->write(1, bits);
                break;
            case ':':
                b->write(2, bits);
                break;
            case '-':
                b->write(3, bits);
                break;
            case '#':
                b->write(4, bits);
                break;   
        }
    }
}

void 
decode_separators(std::vector<char>& sep, const std::shared_ptr<BitBuffer>& b, int num)
{
    const char* smap = " .:-#";
    while (num --> 0)
        sep.push_back(smap[b->read(3)]);
}

void
decode(std::string ifilename, std::string ofilename = std::string())
{ 
    std::cout << "[ STARTING DECOMPRESSION ]\n";
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    b->read_from_file(ifilename);
    int num_fields = b->read(8);
    int entries = b->read(24);
    std::cout << "  - Entries: " << entries << "\n";
    std::vector<MetadataFieldEncoder*> fields;
    for (int i = 0; i < num_fields; i++)
    {
        MetadataFieldEncoder *enc;
        int fieldtype = b->read(2);
        //std::cout << "Fieldtype: " << fieldtype << "\n";
        switch (fieldtype)
        {
            case 0: 
            {
                enc = new ConstantAlphanumericFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break; 
            }
            case 1:
            {
                enc = new AlphanumericFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
            case 2:
            {
                enc = new NumericFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
            case 3:
            {
                enc = new AutoIncrementingFieldEncoder(b);
                enc->decode_metadata();
                fields.push_back(enc);
                break;
            }
        }
    }
    std::vector<char> sep;
    decode_separators(sep, b, num_fields - 1);
    sep.push_back('\n');
    std::ofstream of;
    std::streambuf *buf;
    if (ofilename.empty())
    {
        buf = std::cout.rdbuf();
    }
    else
    {
        of.open(ofilename);
        buf = of.rdbuf();
    }
    std::ostream os(buf);
    while (entries --> 0)
    {
        //os << "@";
        for (int i = 0; i < num_fields; i++)
        {
            fields[i]->decode(os);
            os << sep[i];
        }
    }
    std::cout << "[ FINISHED DECOMPRESSION ]\n";
}

void 
encode(std::string ifilename, std::string ofilename, int entries = 1)
{
    std::cout << "[ STARTING COMPRESSION ]\n";
    std::cout << "  - Entries: " << entries << "\n";
    // Encode sequence identifiers metadata
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    std::ifstream file(ifilename, std::ifstream::in);
    std::vector<MetadataFieldEncoder*> fields;
    std::vector<char> sep;
    
    metadata_analyze(file, sep, fields, entries, b);
    
    for (auto it = fields.begin(); it != fields.end(); it++)
        (*it)->encode_metadata();

    encode_separators(sep, b);

    file.clear();
    file.seekg(0, std::ios::beg);

    // Compress sequence identifiers!       
    int num_fields = fields.size();
    std::string metadata;
    for (int rem = 0; rem < entries && std::getline(file, metadata); rem++)
    {
        if (entries >= 100 && rem % (entries / 100) == 0)
            printf("\r  - Compressing [%3d%]", rem * 100 / entries);
        metadata.erase(0,1);
        std::vector<std::string> v;
        split(metadata, v);       
        for (int i = 0; i < num_fields; i++)
            fields[i]->encode(v[i]);
        std::getline(file, metadata);
        std::getline(file, metadata);
        std::getline(file, metadata);      
    }
    b->write_to_file(ofilename);
    std::cout << "\r  - Compressing [100%]\n  - Compressed size: " << b->size() << " bytes\n";
    
    // Cleanup
    for (auto it = fields.begin(); it != fields.end(); it++)
    {
        delete *it;
    }
    file.close();
    std::cout << "[ FINISHED COMPRESSION ]\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: metadataencoder [--encode|--decode] [FILE] [entries]" << std::endl;
        return -1;
    }
    int rows = argc >= 4 ? atoi(argv[3]) : 1000;
    std::string ifn(argv[2]);
    std::string ofn(ifn);
    if (strncmp(argv[1], "--encode", 8) == 0)
    {
        ofn.append(".seqid.cmprs");
        encode(ifn, ofn, rows);
    } 
    else if (strncmp(argv[1], "--decode", 8) == 0)
    {  
        ofn.erase(ofn.end()-5, ofn.end());
        decode(ifn, ofn);
    } 
    
    //decode(ofn);
}
