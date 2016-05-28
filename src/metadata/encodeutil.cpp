#include "encodeutil.hpp"


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