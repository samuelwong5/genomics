#include "rl_encoder.hpp"
#include <iostream>
#include <cmath>

RlEncoder::RlEncoder(std::string const& alphabet, uint8_t count_length)
{
    offset = ((uint8_t) alphabet[0]) - 1; // Leave 0 for stopcode
    enc_char_length = (uint8_t) ceil(log(alphabet.length() + 1) / log(2));
    enc_count_length = count_length;
    enc_count_max = 100; //pow(2, count_length) + 2;
    for (int i = 0; i < 100; i++)
        occur[i] = 0;
}

std::shared_ptr<BitBuffer> RlEncoder::encode(std::string const& plaintext)
{
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    uint32_t i = 0;
    while (i < plaintext.length()) {
        char c = plaintext[i];
        uint32_t count = 1;
        while (plaintext[++i] == c && i < plaintext.length() && count < enc_count_max) {
            count++;
        }
        b->write(((uint32_t) c) - offset, enc_char_length);
        if (count == 1)
            b->write(0, 1);
        else if (count > 1) {
            b->write(1, 1);
            b->write(count - 2, enc_count_length); 
        }
        occur[count - 1]++;
    }
    return b;
}

std::string RlEncoder::decode(std::shared_ptr<BitBuffer> b)
{
    std::string plaintext;
    while (!b->read_is_end()) {
        char c = b->read(enc_char_length);
        if (b->read(1) == 1) {
            uint32_t count = b->read(enc_count_length);
            plaintext.append(count + 2, c + offset);
        } else {
            plaintext.append(1, c + offset);
        }
    }
    return plaintext;
}

void RlEncoder::print_occur(void)
{
    for (int i = 0; i < 100; i++)
        std::cout << i + 1 << ": " << occur[i] << std::endl;
}

int mainnot()
{
    std::string plaintext = "AAAABBBBCCDDABCBABDBAAAAAABBBDBDBCBABFGGGGGGGG";
    RlEncoder rl ("ABCDEFG", 4);
    std::shared_ptr<BitBuffer> b = rl.encode("AAAABBBBCCDDABCBABDBAAAAAABBBDBDBCBABFGGGGGGGG");
    std::cout << plaintext << std::endl;
    b->print();
    std::string decoded = rl.decode(b);
    std::cout << decoded << std::endl;
    return 0;
}