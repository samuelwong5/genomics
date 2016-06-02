#include <iostream>
#include "lzw_encoder.hpp"

LZWEncoder::LZWEncoder(std::string const& alphabet)
{
    dict = std::vector<std::string> (1, "");
    alphabet_size = alphabet.length();
    for (uint32_t i = 0; i < alphabet.length(); i++) {
        dict.push_back(std::string(1, alphabet[i]));
    }
}

std::shared_ptr<BitBuffer> LZWEncoder::encode(std::string const& plaintext)
{
    reset();
    std::string input = std::string(plaintext);
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    while (!input.empty()) {
        std::vector<std::string>::reverse_iterator rit = dict.rbegin();
        std::string match;
        int code = dict.size();
        for (; rit != dict.rend(); ++rit) {
            code--;
            if (input.compare(0, rit->length(), *rit) == 0) {
                match = *rit;
                break;
            }
        }
        
        int len = match.length();
        b->write(code, enc_size);
        std::string new_entry = std::string(input, 0, len + 1);
        dict.push_back(new_entry);
        input.erase(0, len);
        if (dict.size() >= max_entry) {
            enc_size++;
            max_entry *= 2;
        }
    }
    b->write(0, enc_size);
    return b;
}

std::string LZWEncoder::decode(std::shared_ptr<BitBuffer> b)
{
    reset();
    std::string plaintext;
    std::string last_decoded;
    uint8_t code = b->read(enc_size);
    while (code > 0) {
        if (code >= dict.size()) { // Edge case when cScSc where c is a character and S is a std::string
            last_decoded.append(1, last_decoded[0]);
            dict.push_back(last_decoded);
            plaintext.append(last_decoded);
        } else {
            std::string curr = dict.at(code);
            plaintext.append(curr);
            if (!last_decoded.empty()) {
                last_decoded.append(1, curr[0]);
                dict.push_back(last_decoded);
            }
            last_decoded = std::string(curr);
        }
        if (dict.size() + 1 >= max_entry) {
            enc_size++;
            max_entry *= 2;
        }
        code = b->read(enc_size);
    }
    return plaintext;
}

void LZWEncoder::reset(void)
{
    dict.resize(alphabet_size);
    enc_size = (uint8_t) ceil(log(alphabet_size) / log(2));
    max_entry = (uint32_t) pow(2, enc_size);
}
