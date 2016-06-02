#include "encodeutil.hpp"

void
EncodeUtil::split(std::string& str, std::vector<std::string>& parts) {
  size_t start, end = 0;
  static std::string delim = (" #.-:");
  while (end < str.size()) {
    start = end;
    while (start < str.size() && (delim.find(str[start]) != std::string::npos)) {
      start++;
    }
    end = start;
    while (end < str.size() && (delim.find(str[end]) == std::string::npos)) {
      end++;
    }
    if (end-start != 0) {
      parts.push_back(std::string(str, start, end-start));
    }
  }
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
int 
EncodeUtil::ceil_log(int value, int base)
{
    int power = 1;
    int curr = base;
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
   if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char *p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}