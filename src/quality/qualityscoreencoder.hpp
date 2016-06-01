#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../bitbuffer/bitbuffer.hpp"
#include "../sequence/reads.hpp"

class QualityScoreEncoder {
  private:
    static const uint8_t FREQUENCY_BITS  = 32;
    static const uint8_t VALUE_BITS      = 32;
    static const uint64_t MAX_VALUE     = (uint64_t(1) << VALUE_BITS) - 1;
    static const uint64_t MAX_FREQ      = (uint64_t(1) << FREQUENCY_BITS) - 1;
    static const uint64_t ONE_FOURTH    = uint64_t(1) << (VALUE_BITS - 2);;
    static const uint64_t ONE_HALF      = 2 * ONE_FOURTH;
    static const uint64_t THREE_FOURTHS = 3 * ONE_FOURTH;
    static const int BASE_ALPHABET_SIZE  = 44;
    static const int MAX_CONSEC_ZERO     = 20;
    static const int SYMBOL_SIZE         = BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO + 1; // Last is EOF
    uint64_t frequency[SYMBOL_SIZE];
    std::shared_ptr<BitBuffer> b;
    int pending_bits = 0;
    uint64_t high = MAX_VALUE;
    uint64_t low = 0;
    bool freeze = false;                               // Stop updating frequency table
    
  public:
    QualityScoreEncoder();
    void reset(void);
    void encode_symbol(uint32_t);
    void encode_entry(std::string);
    void decode_entry(std::ofstream&, int);
    void update(int);
    void encode_flush(void);
    void qualityscore_compress(std::vector<read_t>&, char*);
};