#include "encodeutil.hpp"


void
EncodeUtil::split(std::string& str, std::vector<std::string>& parts) {
    size_t start = 0, end = 0;
  
    // Delimiter characters
    static std::string delim = (" #.-:");
 
    while (start < str.size()) {
        end = start;
        while (end < str.size() && (delim.find(str[end]) == std::string::npos))
            end++;
        parts.push_back(std::string(str, start, end-start));
        start = end + 1;
    }
  
    // Edge case: empty value for last field
    if (start == str.size())
        parts.push_back("");
}


void
EncodeUtil::bb_entry(uint32_t val, uint8_t len, std::shared_ptr<std::vector<bb_entry_t> >& entries)
{
    bb_entry_t bbe;
    bbe.value = val;
    bbe.length = len;
    entries->push_back(bbe);
}

/*  ---------------------------------------------
 *  ceil_log
 *  ---------------------------------------------
 *  Calculates the ceiling of the log of a given
 *  number and given base.
 * 
 *  Parameters:
 *    int value - integer value 
 *    int j     - integer base
 *
 *  Returns:
 *    int       - result of ceil(log(value)/log(j))
 */
uint32_t
EncodeUtil::ceil_log(uint32_t value, uint32_t base)
{ 

    uint32_t power = 1;
    uint32_t curr = base;
    while (curr <= value)
    {
        power++;
        curr *= base;
    }
    return power;
}


/*  ---------------------------------------------
 *  is_numeric
 *  ---------------------------------------------
 *  Finds whether a given string is an _integer_
 *  value.
 *
 *  Parameters:
 *    const string& s  - input string
 
 *  Returns:
 *    bool             - whether input is numeric
 */
bool 
EncodeUtil::is_numeric(const std::string & s)
{
   if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

   char *p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}