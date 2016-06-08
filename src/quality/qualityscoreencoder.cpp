#include "qualityscoreencoder.hpp"


QualityScoreEncoder::QualityScoreEncoder()
{
    b = std::shared_ptr<BitBuffer>(new BitBuffer);
    frequency = new uint64_t[SYMBOL_SIZE];
    if (!frequency)
    {
        std::cout << "[FATAL] Unable to allocate memory for Quality Score frequency table.\n";
        exit(-1);
    }
    reset();
}

static bool freeze = false;
void
QualityScoreEncoder::update(int c, uint64_t *freq)
{
    if (!freeze)
        freq[c+1]++;
}


void
QualityScoreEncoder::reset(void) 
{
    freeze = false;
    for (int i = 0; i < SYMBOL_SIZE; i++)
        frequency[i] = i;
}


void
QualityScoreEncoder::decode_entry(read_t& r)
{
    char *qs = r.q_score;
    char prev = 0;    
    uint64_t pcount = frequency[SYMBOL_SIZE - 1];
    for (uint32_t i = 0; i < entry_len;) {
      uint64_t range = high - low + 1;
      uint64_t scaled_value =  ((value - low + 1) * pcount - 1 ) / range;

      int c = 0;
      for (int i = 0; i < SYMBOL_SIZE - 1; i++)
      {
          if (scaled_value < frequency[i+1]) 
          {
             c = i;  
             break;
          }
      }

      // Deconstruct 3-ary grouping
      int c3 = c % ALPHABET_SIZE;
      c /= SYMBOL_SIZE;
      int c2 = c % ALPHABET_SIZE;
      int c1 = c / ALPHABET_SIZE;
      //printf("<%d, %d, %d>\n", c1, c2, c3);
      // Decode 3-ary grouping
      if (c1 >= BASE_ALPHABET_SIZE)
      {
          i += c1 - BASE_ALPHABET_SIZE + 1;
          for (int j = 0; j < c1 - BASE_ALPHABET_SIZE + 1; j++)
              *qs++ = prev;
      }
      else
      {
          i++;
          prev = c1 + BASE_CHAR_OFFSET;
          *qs++ = prev;
      }
      if (i >= entry_len) { break;}
      if (c2 >= BASE_ALPHABET_SIZE)
      {
          i += c2 - BASE_ALPHABET_SIZE + 1;
          for (int j = 0; j < c2 - BASE_ALPHABET_SIZE + 1; j++)
              *qs++ = prev;
      }
      else
      {
          i++;
          prev = c2 + BASE_CHAR_OFFSET;
          *qs++ = prev;
      }
      if (i >= entry_len) { break;}
      if (c3 >= BASE_ALPHABET_SIZE)
      {
          i += c3 - BASE_ALPHABET_SIZE + 1;
          for (int j = 0; j < c3 - BASE_ALPHABET_SIZE + 1; j++)
              *qs++ = prev;
      }
      else
      {
          i++;
          prev = c3 + BASE_CHAR_OFFSET;
          *qs++ = prev;
      }
      if (i >= entry_len) { break;}

      // Find next value
      uint32_t phigh = frequency[c + 1];
      uint32_t plow = frequency[c];
     
      high = low + (range*phigh)/pcount -1;
      low = low + (range*plow)/pcount;
      if (c == SYMBOL_SIZE - 1)
      {
          std::cout << "REACHED EOF!\n";
          break;
      }
      for( ; ; ) {
        if ( high < ONE_HALF ) {
          // Do nothing
        } else if ( low >= ONE_HALF ) {
          value -= ONE_HALF;  
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
    }
    *qs = '\0';
}


void 
QualityScoreEncoder::encode_symbol(uint32_t c)
{
    //int c_c = c;
    //int c3 = c_c % ALPHABET_SIZE;
    //c_c /= SYMBOL_SIZE;
    //int c2 = c_c % ALPHABET_SIZE;
    //int c1 = c_c / ALPHABET_SIZE;
    //printf("<%d, %d, %d>\n", c1, c2, c3);

    if (c == SYMBOL_SIZE - 1)
        std::cout << "  - Done encoding quality scores.\n";
    uint32_t phigh = frequency[c+1];
    uint32_t plow = frequency [c];
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
            if (pending_bits > 32)
                std::cout<<"shit. ";
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
QualityScoreEncoder::translate_symbol(std::vector<read_t>::iterator begin, std::vector<read_t>::iterator end, std::vector<uint32_t>& symbols, uint64_t* freq)
{
    uint32_t consec = 0;
    char prev = 0;
    // Ignore last '\0'
    uint32_t len = begin->seq_len;
    uint32_t symbol = 0;
    uint32_t symbol_index = 0;
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
                    symbol *= ALPHABET_SIZE;
                    symbol += BASE_ALPHABET_SIZE + MAX_CONSEC_ZERO - 1;
                    symbol_index++;
                    if (symbol_index % TUPLE_SIZE == 0)
                    {
                        update(symbol, freq);
                        symbols.push_back(symbol);
                        symbol = 0;
                        symbol_index = 0;
                    }
                    consec = 0;
                }
            }
            else
            {
                if (consec > 0)
                {
                    symbol *= ALPHABET_SIZE;
                    symbol += BASE_ALPHABET_SIZE + consec - 1;
                    symbol_index++;  
                    if (symbol_index % TUPLE_SIZE == 0)
                    {
                        update(symbol, freq);
                        symbols.push_back(symbol);
                        symbol = 0;
                        symbol_index = 0;
                    }
                    consec = 0;
                }
                symbol *= ALPHABET_SIZE;
                symbol += curr - BASE_CHAR_OFFSET;
                symbol_index++;  
                if (symbol_index % TUPLE_SIZE == 0)
                {
                    update(symbol, freq);
                    symbols.push_back(symbol);
                    symbol = 0;
                    symbol_index = 0;
                }
                prev = curr;
            }
        }
        if (consec > 0)
        {
            symbol *= ALPHABET_SIZE;
            symbol += BASE_ALPHABET_SIZE + consec - 1;
            symbol_index++;  
            consec = 0;
        }
        while (symbol_index > 0 && symbol_index++ < TUPLE_SIZE) { symbol *= ALPHABET_SIZE; }
        update(symbol, freq);
        symbols.push_back(symbol);
        symbol = 0;
        symbol_index = 0;
        prev = 0;
    }
}


QualityScoreEncoder::QualityScoreEncoder(char *filename)
{
    b = std::shared_ptr<BitBuffer>(new BitBuffer);
    std::string ifn(filename);
    ifn.append(".qs");
    b->read_from_file(ifn);

    frequency = new uint64_t[SYMBOL_SIZE];
    if (!frequency)
    {
        std::cout << "[FATAL] Unable to allocate memory for Quality Score frequency table." << std::endl;;
        exit(-1);
    }
    reset();
}

static uint32_t MAGIC_NUMBER = 0x12345678;

void
QualityScoreEncoder::qualityscore_decompress(std::vector<read_t>& reads, char *filename)
{
    std::cout << " [QUALITY SCORES]\n";
    uint32_t curr_entry = 0;
    b->read_pad_back();
    uint32_t match_magic = 0;
    std::cout << "Matching magic number... ";
    while (MAGIC_NUMBER != match_magic)
    {
        match_magic = b->read(32);
    }
    std::cout << "MATCHED!\n";
    uint32_t entries = b->read(32);
    entry_len = b->read(32);

    std::cout << " - Entries: " << reads.size() << "  len: " << entry_len <<  std::endl;       
    //if (reads.size() < entries)
    //    reads.resize(entries);
    
    if (!freeze) {
        std::cout << " - Reading frequencies..\n";
        // Read in frequencies
        for (int i = 0; i < SYMBOL_SIZE; i++)
        {
            frequency[i] = b->read(32);
            if ((i % 50000) == 0)
                std::cout << i << ": " << frequency[i] << std::endl;    
        }
        freeze = true;
    }    
    value = b->read(VALUE_BITS);
    pending_bits = 0;
    high = MAX_VALUE;
    low = 0;
    std::cout << " - Decoding..\n";
    // Decode
    for (uint32_t i = 0; i < entries; i++)
    {
 //     std::cout << "Entry: " << i << std::endl;
        decode_entry(reads[curr_entry++]);
    }
}


void
QualityScoreEncoder::qualityscore_compress(std::vector<read_t>& reads, char* filename)
{
    std::cout << " [QUALITY SCORES]" << std::endl;;
    b->init();
    b->write(MAGIC_NUMBER, 32);
    int entries = reads.size();
    b->write(entries, 32);
    int entry_len = reads[0].seq_len;
    b->write(entry_len, 32);

    
    // Translate into run-length symbols
    std::cout << "  - Translating symbols" << std::endl;;
    std::vector<uint32_t> symbols[N_THREADS];
    
    uint64_t* frequencies[N_THREADS];
    for (int i = 0; i < N_THREADS; i++)
    {
        frequencies[i] = new uint64_t[SYMBOL_SIZE]();
        if (!frequencies[i])
        {
            std::cout << "[FATAL] Unable to allocate memory for Quality Score frequencies table.\n";
            exit(-1);
        }
    }

#pragma omp parallel for num_threads(N_THREADS)    
    for (int i = 0; i < N_THREADS; i++)
    {
        for (int j = 0; j < SYMBOL_SIZE; j++)
            frequencies[i][j] = 0;
        int boffset = entries * i / N_THREADS;
        int eoffset = entries * (i+1) / N_THREADS;
        translate_symbol(reads.begin() + boffset, reads.begin() + eoffset, symbols[i], frequencies[i]);
        //printf("    - Thread %d : %lu symbols.\n", i, symbols[i].size());
        //for (int j = 0; j < SYMBOL_SIZE; j++)
       // {
       //     if ((j % 50000) == 0)
       //         std::cout << i << " thread " << j << ": " << frequencies[i][j] << std::endl;
       // }   
    }
    
    if (!freeze) {
        std::cout << "  - Calculating frequency table" << std::endl;
        // Sum frequencies and write to file
       // printf("Sym size: %d\n", SYMBOL_SIZE);
        for (int i = 0; i < SYMBOL_SIZE; i++)
        {
            for (int j = 0; j < N_THREADS; j++)
                frequency[i] += frequencies[j][i];
        }  
        // Transform to cumulative
        for (int i = SYMBOL_SIZE - 1; i >= 0; i--)
            for (int j = i - 1; j >= 0; j--)
                frequency[i] += frequency[j];
        
        for (int j = 0; j < SYMBOL_SIZE; j++)
        {
            b->write(frequency[j], 32);
            if ((j % 1000) == 0)
                std::cout << "Cumalative " << j << ": " << frequency[j] << std::endl;
        }   
    }

    // Arithmetic encoding for run-length symbols
    std::cout << "  - Encoding symbols" << std::endl;;
    high = MAX_VALUE;
    low = 0;
    pending_bits = 0;
    int symbol_cnt = 0;
    for (int i = 0; i < N_THREADS; i++)
    {
        for (auto it = symbols[i].begin(); it != symbols[i].end(); it++)
            encode_symbol(*it);
        symbol_cnt += symbols[i].size();
    }
    encode_flush();

    // Write to file and cleanup
    std::string ofilename(filename);
    ofilename.append(".qs");
    b->write_to_file(ofilename);
    for (int i = 0; i < N_THREADS; i++)
        delete[] frequencies[i];

    std::cout << "  - Compressed size: " << b->size() << " bytes (Symbol count: " << symbol_cnt << ")\n";
}
