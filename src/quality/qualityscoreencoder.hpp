#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../bitbuffer/bitbuffer.hpp"
#include "../sequence/reads.hpp"


class QualityScoreEncoder {
  private:
    // Arithmetic encoding constants
    static const uint8_t FREQUENCY_BITS  = 32;
    static const uint8_t VALUE_BITS      = 32;
    static const uint64_t MAX_VALUE      = (uint64_t(1) << VALUE_BITS) - 1;
    static const uint64_t MAX_FREQ       = (uint64_t(1) << FREQUENCY_BITS) - 1;
    static const uint64_t ONE_FOURTH     = uint64_t(1) << (VALUE_BITS - 2);;
    static const uint64_t ONE_HALF       = 2 * ONE_FOURTH;
    static const uint64_t THREE_FOURTHS  = 3 * ONE_FOURTH;
    
    // Arithmetic encoding values
    int pending_bits                     = 0;
    uint64_t high                        = MAX_VALUE;
    uint64_t low                         = 0;
    uint64_t value                       = 0;
    
    // Domain of inputs
    static const int BASE_ALPHABET_SIZE  = 44;
    static const int BASE_CHAR_OFFSET    = 33;
    static const int MAX_CONSEC_ZERO     = 60;
    static const int SYMBOL_SIZE         = BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO + 1; // Last is EOF
    
    uint64_t frequency[SYMBOL_SIZE];                   // Frequencies to calculate encoding
    std::shared_ptr<BitBuffer> b;                      // Buffer for file IO
    bool freeze = false;                               // Stop updating frequency table
    uint32_t entry_len;
    
  public:
    QualityScoreEncoder();
    QualityScoreEncoder(char *);
    void reset(void);
    void qualityscore_compress(std::vector<read_t>&, char*);
    void qualityscore_decompress(std::vector<read_t>&, char*);
    inline void update(int, uint64_t*);
    void translate_symbol(std::vector<read_t>::iterator, std::vector<read_t>::iterator, std::vector<uint8_t>&, uint64_t*);
    void encode_symbol(uint32_t);
    void encode_flush(void);
    void decode_entry(read_t&);

};
