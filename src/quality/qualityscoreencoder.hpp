#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "bitbuffer.hpp"

class QualityScoreEncoder {
  private:
    static const uint8_t FREQUENCY_BITS  = 32;
    static const uint8_t VALUE_BITS      = 32;
    static const uint64_t MAX_VALUE     = (uint64_t(1) << VALUE_BITS) - 1;
    static const uint64_t MAX_FREQ      = (uint64_t(1) << FREQUENCY_BITS) - 1;
    static const uint64_t ONE_FOURTH    = uint64_t(1) << (VALUE_BITS - 2);;
    static const uint64_t ONE_HALF      = 2 * ONE_FOURTH;
    static const uint64_t THREE_FOURTHS = 3 * ONE_FOURTH;
    static const int SYMBOL_SIZE         = 45; // 0-43 characters + 44 total
    uint32_t frequency[45];
    std::shared_ptr<BitBuffer> b;
    
  public:
    QualityScoreEncoder(std::shared_ptr<BitBuffer>);
    void reset(void);
    void encode_entry(std::string);
    void update(int);
};