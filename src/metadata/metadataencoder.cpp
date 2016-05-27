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

int ceil_log(int max, int base)
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

bool isNumeric(const std::string & s)
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

void split(std::string& str, std::vector<std::string>& parts) {
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
metadata_analyze(std::ifstream &file, std::vector<char>& sep, std::vector<MetadataFieldEncoder>& fields, int entries)
{   
    std::string metadata;
    int remaining = entries - 1;
    std::getline(file, metadata);
    metadata.erase(0,1);
    metadata_separators(metadata, sep);    // List of Separators in order  
    int num_fields = sep.size() + 1;
    std::cout << "Fields: " << num_fields << std::endl;
    
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
    
    
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    for (auto it = values.begin(); it != values.end(); it++)
    {
        std::string str("1");
        if (it->size() == 1)
        {
            std::cout << "Constant Alphanumeric: " << *it->begin() << std::endl;
            fields.push_back(ConstantAlphanumericFieldEncoder(b, *it->begin()));
        }
        else if (it->size() > 0)
        {
            int max = 0;
            //int prev = -1;
            //int max_delta = 0;
            bool numeric = true;
            for (auto sit = it->begin(); sit != it->end(); sit++)
            {
                if (!numeric && !isNumeric(*sit))
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
                    std::cout << "Auto Incrementing" << std::endl;    
                    fields.push_back(AutoIncrementingFieldEncoder(b, 1));
                }
                else 
                {
                    std::cout << "Numeric  (Values: " << it->size() << " |  Max: " << max << ")\n";
                    // Non-incremental
                    fields.push_back(NumericFieldEncoder(b, ceil_log(max + 1, 2), false));
                }
            }        
            else 
            {
                std::cout << "Alphanumeric  (Values: " << it->size() << ")\n";
                int bits_per_char = 8;
                fields.push_back(AlphanumericFieldEncoder(b, max * bits_per_char, false, std::set<std::string>()));
                break;
            }
        }
    }
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: metadataencoder [filename]" << std::endl;
        return -1;
    }
    std::ifstream ifs(argv[1], std::ifstream::in);
    int rows = argc >= 3 ? atoi(argv[2]) : 100000;
    std::vector<MetadataFieldEncoder> fields;
    std::vector<char> sep;
    metadata_analyze(ifs, sep, fields, rows);
}
