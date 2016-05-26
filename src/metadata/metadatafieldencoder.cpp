#include "metadatafieldencoder.hpp"

int tokens = 10;

MetadataFieldEncoder::MetadataFieldEncoder(std::shared_ptr<BitBuffer> b) : buffer(b)
{

}

/*
std::array<int, tokens> tokenize(std::string metadata) {
    const std::string sep = "@. :#/";
    std::array<int, tokens> tokens;
    int index = 0;
    int curr = 0;
    for ( std::string::iterator it=str.begin(); it!=str.end(); ++it) {
        if (sep.find(*it) == string::npos)
            curr = curr * 10 + (*it - '0');
        else if (*it > '9' || *it < '0') {
            curr = 0;
            while (sep.find(*it) != string::npos) { it++; }
        } else {
            tokens[index++] = curr;
            curr = 0;
        }
    }
    tokens[index++] = curr;
    return tokens;
}*/