#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../utils/bitbuffer.hpp"
#include "../sequence/reads.hpp"

class QualityScoreEncoder {
  private:
    // Magic number for error checking 
    static const uint32_t MAGIC_NUMBER          = 0x12345678;
  
    // Arithmetic encoding constants
    static const uint8_t FREQUENCY_BITS         = 32;
    static const uint8_t VALUE_BITS             = 32;
    static const uint64_t MAX_VALUE             = (uint64_t(1) << VALUE_BITS) - 1;
    static const uint64_t MAX_FREQ              = (uint64_t(1) << FREQUENCY_BITS) - 1;
    static const uint64_t ONE_FOURTH            = uint64_t(1) << (VALUE_BITS - 2);
    static const uint64_t ONE_HALF              = 2 * ONE_FOURTH;
    static const uint64_t THREE_FOURTHS         = 3 * ONE_FOURTH;
    
    // Arithmetic encoding values
    int pending_bits                            = 0;
    uint64_t high                               = MAX_VALUE;
    uint64_t low                                = 0;
    uint64_t value                              = 0;
    uint32_t THRESHOLD_PER_TEN                  = 40;  // Threshold for recalculating frequency table
    
    // Domain of inputs
    static const uint32_t BASE_ALPHABET_SIZE    = 128;
    static const uint32_t BASE_CHAR_OFFSET      = 33;
    static const uint32_t MAX_CONSEC_ZERO       = 60;
    static const uint32_t SYMBOL_SIZE           = BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO + 1; 
    
    std::shared_ptr<BitBuffer> b;                      // Buffer for file IO    
    uint64_t frequency[SYMBOL_SIZE];                   // Frequencies to calculate encoding
    bool freeze = false;                               // Stop updating frequency table
    uint32_t entry_len;                                // Length of each quality score entry
    
  public:
    QualityScoreEncoder(void);
    QualityScoreEncoder(char *);                       
    QualityScoreEncoder(uint64_t*);
    
    // Frequency table
    void reset(void);
    inline void update(int, uint64_t*);
    
    // Compress functions
    void qualityscore_compress(std::vector<read_t>&, char*);
    void translate_symbol(std::vector<read_t>::iterator, std::vector<read_t>::iterator, std::vector<uint8_t>&, uint64_t*);
    std::shared_ptr<BitBuffer> get_bb(void);
    std::shared_ptr<BitBuffer> compress_parallel(std::vector<uint8_t>&);
    void encode_symbol(uint32_t);
    void encode_flush(void);
    void encode_magic(void);
    void set_encode_threshold(uint32_t);
    
    // Decompress functions
    void qualityscore_decompress(std::vector<read_t>&, char*);
    void decode_entry(read_t&);
};
