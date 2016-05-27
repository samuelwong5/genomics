#include <fstream>
#include <vector>
#include <iostream>
#include <set>
#include <cstring>
#include <cstdlib>
#include <cmath>

bool isNumeric(const std::string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

std::vector<char>
init(std::string metadata)
{
    std::vector<char> separators;
    for (std::string::iterator it = metadata.begin(); it != metadata.end(); ++it)
    {
        if (*it == ' ' || *it == '.' || *it == '-' || *it == ':' || *it == '#')
            separators.push_back(*it);
    }
    return separators;
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
metadata_analyze(std::ifstream &file, int entries)
{    
    std::string metadata;
    int remaining = entries - 1;
    std::getline(file, metadata);
    metadata.erase(0,1);
    std::vector<char> sep = init(metadata);    // List of Separators in order  
    int fields = sep.size() + 1;
    std::cout << "Fields: " << fields << std::endl;
    // Initialize set of values for each field to count the number of 
    // total values
    std::vector<std::set<std::string> > values;
    for (int i = 0; i < fields; i++)
        values.push_back(std::set<std::string>());
    {
        std::vector<std::string> v;
        split(metadata, v);
        for (int i = 0; i < fields; i++)
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
        for (int i = 0; i < fields; i++)
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
            std::cout << "Constant Alphanumeric" << std::endl;
        }
        else if (it->size() > 0)
        {
            int max = 0;
            int prev = -1;
            int max_delta = 0;
            bool numeric = true;
            for (auto sit = it->begin(); sit != it->end(); sit++)
            {
                if (!isNumeric(*sit))
                {
                    numeric = false;
                    std::cout << "Alphanumeric  (Values: " << it->size() << ")\n";
                    break;
                }
                else
                {
                    int val = atoi(sit->c_str());
                    max = max > val ? max : val;
                    prev = val;                        
                }
            }
            if (numeric)
            {
                if (max == entries)
                    std::cout << "Auto Incrementing" << std::endl;    
                else 
                    std::cout << "Numeric  (Values: " << it->size() << " |  Max: " << max << ")\n";
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
    int rows = 100000;
    metadata_analyze(ifs, rows);
}
