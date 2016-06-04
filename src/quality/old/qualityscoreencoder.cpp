#include "qualityscoreencoder.hpp"

QualityScoreEncoder::QualityScoreEncoder()
{
    reset();
    b = std::shared_ptr<BitBuffer>(new BitBuffer);
}

void
QualityScoreEncoder::update(int c, uint64_t *freq)
{
    for (int i = c + 1; i < SYMBOL_SIZE; i++)
        freq[i]++;
}

void
QualityScoreEncoder::reset(void) 
{
    freeze = false;
    for (int i = 0; i < SYMBOL_SIZE; i++)
        frequency[i] = i;
}

/*
void
QualityScoreEncoder::decode_entry(std::ofstream& output, int entry_len)
{
    static const int BASE_CHAR_OFFSET = 33;
    static uint64_t high = MAX_VALUE;
    static uint64_t low = 0;
    static uint64_t value = b->read(VALUE_BITS);
    static char prev = -1;
    for (int i = 0; i < entry_len; i++) {
      uint64_t range = high - low + 1;
      uint32_t pcount = frequency[SYMBOL_SIZE - 1];
      uint64_t scaled_value =  ((value - low + 1) * pcount - 1 ) / range;
      //std::cout << scaled_value << "\n";
      int c = SYMBOL_SIZE - 2;
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
      //std::cout << frequency[c - 1] << " < " << plow << " < " << scaled_value << " (" << c << ") < " << phigh << std::endl;
      //std::cout << c << " ";
      if (c >= BASE_ALPHABET_SIZE)
      {
          i += c - BASE_ALPHABET_SIZE;
          for (int j = 0; j < c - BASE_ALPHABET_SIZE + 1; j++)
              output << prev;
      }
      else
      {

          prev = c + BASE_CHAR_OFFSET;
          output << prev;
      }
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
}*/

void 
QualityScoreEncoder::encode_symbol(uint32_t c)
{
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
}

/*
void 
QualityScoreEncoder::encode_entry(std::string qs)
{
    uint32_t prev = -1;
    int consec = 0;
    // Subtract last character which is '\n'
    for (auto it = qs.begin(); it != --qs.end(); it++)
    {
        //std::cout << "  Encoding char: " << *it;
        uint32_t c = *it - BASE_CHAR_OFFSET;
        if (prev == c)
        {
            consec++;
            if (consec >= MAX_CONSEC_ZERO)
            {
                encode_symbol(BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO - 1);
                consec = 0;
            }
        }
        else
        {
            if (consec > 0)
            {
                //std::cout << "Encode consec: " << consec << std::endl;
                encode_symbol(BASE_ALPHABET_SIZE + consec - 1);
                consec = 0;
            }
            //std::cout << "Encode symbol: " << c << std::endl;
            encode_symbol(c);
            prev = c;
        }
    }
    
    if (consec > 0)
        encode_symbol(BASE_ALPHABET_SIZE + consec - 1);
    //std::cout << std::endl;
}
*/
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
QualityScoreEncoder::translate_symbol(std::vector<read_t>::iterator begin, std::vector<read_t>::iterator end, std::vector<uint8_t>& symbols, uint64_t* freq)
{
    uint32_t consec = 0;
    char prev = 0;
    // Ignore last '\0'
    uint32_t len = begin->seq_len - 1;
    for (auto it = begin; it != end; it++)
    {
        char *entry = it->q_score;
        for (uint8_t i = 0; i < len; i++)
        {
            char curr = entry[i];
            if (prev == curr)
            {
                consec++;
                if (consec >= MAX_CONSEC_ZERO)
                {
                    update(BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO - 1, freq);
                    symbols.push_back(BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO - 1);
                    consec = 0;
                }
            }
            else
            {
                if (consec > 0)
                {
                    update(BASE_ALPHABET_SIZE + consec - 1, freq);
                    symbols.push_back(BASE_ALPHABET_SIZE + consec - 1);
                    consec = 0;
                }
                update(curr - BASE_CHAR_OFFSET, freq);
                symbols.push_back(curr - BASE_CHAR_OFFSET);
                prev = curr;
            }
        }
        if (consec > 0)
        {
            update(BASE_ALPHABET_SIZE + consec - 1, freq);
            symbols.push_back(BASE_ALPHABET_SIZE + consec - 1);
            consec = 0;
        }
        prev = 0;
    }
}


void
QualityScoreEncoder::qualityscore_compress(std::vector<read_t>& reads, char* filename)
{
    std::cout << " [QUALITY SCORES]\n";
    b->init();
    int entries = reads.size();
    b->write(entries, 32);
    int entry_len = reads[0].seq_len;
    b->write(entry_len, 32);

    std::cout << "  - Translating symbols\n";
    // Translate into run-length symbols
    std::vector<uint8_t> symbols[N_THREADS];
    uint64_t frequencies[N_THREADS][SYMBOL_SIZE];

#pragma omp parallel for num_threads(N_THREADS)    
    for (int i = 0; i < N_THREADS; i++)
    {
        for (int j = 0; j < SYMBOL_SIZE; j++)
            frequencies[i][j] = 0;
        int boffset = entries * i / N_THREADS;
        int eoffset = entries * (i+1) / N_THREADS;
        translate_symbol(reads.begin() + boffset, reads.begin() + eoffset, symbols[i], frequencies[i]);
        //std::cout << "Entries " << boffset << " to " << eoffset << " symbol count: " << symbols[i].size() << "\n";
    }

    // Sum frequencies and write to file
    for (int i = 0; i < SYMBOL_SIZE; i++)
    {
        frequency[i] = 0;
        for (int j = 0; j < N_THREADS; j++)
             frequency[i] += frequencies[j][i];
        b->write(frequency[i], 32);
        //std::cout << "Symbol " << i << ": " << frequency[i] << "\n";
    }        
    
    std::cout << "  - Encoding symbols\n";     
    // Arithmetic encoding for run-length symbols
    for (int i = 0; i < N_THREADS; i++)
        for (auto it = symbols[i].begin(); it != symbols[i].end(); it++)
            encode_symbol(*it);
    encode_flush();

    std::string ofilename(filename);
    ofilename.append(".qs");
    b->write_to_file(ofilename);
    std::cout << "  - Compressed size: " << b->size() << " bytes --> " << ofilename << "\n";
}

/*

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
}*/