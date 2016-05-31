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
}

void
QualityScoreEncoder::reset(void) 
{
    for (int i = 0; i < SYMBOL_SIZE; i++)
        frequency[i] = i;
}

void 
QualityScoreEncoder::encode_entry(std::string qs)
{
    static const int BASE_CHAR_OFFSET = 33;
    int pending_bits = 0;
    uint64_t high = MAX_VALUE;
    uint64_t low = 0;
    for (auto it = qs.begin(); it != --qs.end(); it++)
    {
        uint32_t c = *it - BASE_CHAR_OFFSET;
        update(c);
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
}

void
encode(std::string ifilename, std::string ofilename, int entries = 10000)
{
    std::ifstream input(ifilename, std::ifstream::binary);
    std::shared_ptr<BitBuffer> b(new BitBuffer);   
    QualityScoreEncoder qse(b);
    std::string qs;
    for (int rem = 0; rem < entries && std::getline(input, qs); rem++)
    {
        std::getline(input, qs);
        std::getline(input, qs);
        std::getline(input, qs);
        qse.encode_entry(qs);        
    }
    b->write_to_file(ofilename);
}


int 
main(int argc, char** argv)
{
    if (argc <= 3)
    {
        std::cout << "Usage: qualityscoreencoder [INPUT] [OUTPUT]\n";
        return -1;
    }
    encode(argv[1], argv[2]);
}