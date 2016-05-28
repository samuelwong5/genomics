#ifndef ENCODEUTIL_HPP
#define ENCODEUTIL_HPP

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>


class EncodeUtil {
  public:
    static int ceil_log(int, int);
    static bool is_numeric(const std::string&);
};

#endif
