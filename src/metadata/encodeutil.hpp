#ifndef ENCODEUTIL_HPP
#define ENCODEUTIL_HPP

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "../bitbuffer/bitbuffer.hpp"

class EncodeUtil {
  public:
    static void bb_entry(uint32_t, uint8_t, std::shared_ptr<std::vector<bb_entry_t> >&);
    static int ceil_log(int, int);
    static bool is_numeric(const std::string&);
    static void split(std::string&, std::vector<std::string>&);
};

#endif
