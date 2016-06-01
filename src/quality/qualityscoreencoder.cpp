#include "qualityscoreencoder.hpp"

QualityScoreEncoder::QualityScoreEncoder(std::shared_ptr<BitBuffer> buff) : b(buff)
{
    reset();
}

void
QualityScoreEncoder::update(int c)
{
    for (int i = c + 1; i < SYMBOL_SIZE; i++)
        frequency[i]++;
    if (frequency[SYMBOL_SIZE - 1] >= MAX_FREQ)
        freeze = true;
}

void
QualityScoreEncoder::reset(void) 
{
    freeze = false;
    for (int i = 0; i < SYMBOL_SIZE; i++)
        frequency[i] = i;
}

void
QualityScoreEncoder::decode_entry(std::ofstream& output, int entry_len)
{
    static const int BASE_CHAR_OFFSET = 33;
    static uint64_t high = MAX_VALUE;
    static uint64_t low = 0;
    static uint64_t value = b->read(VALUE_BITS);
    for (int i = 0; i < entry_len; i++) {
      uint64_t range = high - low + 1;
      uint32_t pcount = frequency[SYMBOL_SIZE - 1];
      uint64_t scaled_value =  ((value - low + 1) * pcount - 1 ) / range;
      //std::cout << scaled_value << "\n";
      int c = SYMBOL_SIZE;
      for (int i = 0; i < SYMBOL_SIZE - 1; i++)
      {
          //std::cout << (char) (i + 33) << ":" << frequency[i+1] << std::endl;
          if (scaled_value < frequency[i+1]) 
          {
             //std::cout << "xd: " << i << "\n";
             c = i;  
             break;
          }
      }
      uint32_t phigh = frequency[c + 1];
      uint32_t plow = frequency[c];
      char cr = c + BASE_CHAR_OFFSET;
      output << cr;
      //std::cout << cr;
      high = low + (range*phigh)/pcount -1;
      low = low + (range*plow)/pcount;

      for( ; ; ) {
        if ( high < ONE_HALF ) {
          //do nothing, bit is a zero
        } else if ( low >= ONE_HALF ) {
          value -= ONE_HALF;  //subtract one half from all three code values
          low -= ONE_HALF;
          high -= ONE_HALF;
        } else if ( low >= ONE_FOURTH && high < THREE_FOURTHS ) {
          value -= ONE_FOURTH;
          low -= ONE_FOURTH;
          high -= ONE_FOURTH;
        } else
          break;
        low <<= 1;
        high <<= 1;
        high++;
        value <<= 1;
        value += b->read(1);
      }
      if (!freeze)
          update(c);
      if (c == SYMBOL_SIZE - 1)
          break;
      
    }
    
    output << "\n";
    //std::cout << "\n";
}

void 
QualityScoreEncoder::encode_entry(std::string qs)
{
    //std::cout << "Encoding string: " << qs << std::endl;
    static const int BASE_CHAR_OFFSET = 33;
    for (auto it = qs.begin(); it != qs.end(); it++)
    {
        uint32_t c = *it - BASE_CHAR_OFFSET;
        uint32_t phigh = frequency[c+1];
        uint32_t plow = frequency[c];
        uint64_t range = high - low + 1;
        uint64_t pcount = frequency[SYMBOL_SIZE - 1];
        high = low + (range * phigh / pcount) - 1;
        low = low + (range * plow / pcount);
        for (;;) 
        {
            if (high < ONE_HALF) 
            {
                b->write((1 << pending_bits) - 1, pending_bits + 1);
                pending_bits = 0;
            }
            else if (low >= ONE_HALF)
            {
                b->write(1 << pending_bits, pending_bits + 1);
                pending_bits = 0;
            }
            else if (low >= ONE_FOURTH && high < THREE_FOURTHS)
            {
                pending_bits++;
                low -= ONE_FOURTH;
                high -= ONE_FOURTH;
            } else { break; }
            high <<= 1;
            high++;
            low <<= 1;
            high &= MAX_VALUE;
            low &= MAX_VALUE;
        }
        if (!freeze)
            update(c);
    }
    //std::cout << std::endl;
}

void
QualityScoreEncoder::encode_flush(void)
{
    pending_bits++;
    if (low < ONE_FOURTH)
        b->write((1 << pending_bits) - 1, pending_bits + 1);
    else
        b->write(1 << pending_bits, pending_bits + 1);   
}


void
decode(std::string ifilename, std::string ofilename)
{
    std::shared_ptr<BitBuffer> b(new BitBuffer);   
    b->read_from_file(ifilename);
    QualityScoreEncoder qse(b);
    int entries = b->read(32);
    int entry_len = b->read(32);
    std::cout << "Decoding file " << ifilename << "\n           to " <<    ofilename << "\n    - Entries: " << entries << "\n";
    std::ofstream output;
    output.open(ofilename);
    for (int i = 0; i < entries; i++)
        qse.decode_entry(output, entry_len);
}    

void
encode(std::string ifilename, std::string ofilename, int entries = 10000)
{
    std::cout << "Encoding file " << ifilename << "\n           to " << ofilename << "\n    - Entries: " << entries << "\n";
    std::ifstream input(ifilename);
    std::shared_ptr<BitBuffer> b(new BitBuffer);   
    b->write(entries, 32);         // Write number of entries
    std::string qs;
    for (int i = 0; i < 4; i++)
        std::getline(input, qs);
    b->write(qs.length(), 32); // Write length of each entry
    QualityScoreEncoder qse(b);
    qse.encode_entry(qs);
    for (int rem = 1; rem < entries && std::getline(input, qs); rem++)
    {
        std::getline(input, qs);
        std::getline(input, qs);
        std::getline(input, qs);
        qse.encode_entry(qs);        
    }
    qse.encode_flush();
    b->write_to_file(ofilename);
    //b->print();
}


int 
main(int argc, char** argv)
{
    if (argc <= 3)
    {
        std::cout << "Usage: qualityscoreencoder [--encode|--decode] [INPUT] [OUTPUT]\n";
        return -1;
    }
    int entries = 10000;
    if (argc >= 5)
        entries = atoi(argv[4]);
    if (strncmp(argv[1], "--encode", 8) == 0)
        encode(argv[2], argv[3], entries);
    else if (strncmp(argv[1], "--decode", 8) == 0)
        decode(argv[2], argv[3]);
}