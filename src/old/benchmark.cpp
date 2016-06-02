#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "lzw_encoder.hpp"
#include "rl_encoder.hpp"

int main(void) 
{
    //LZWEncoder zlzw ("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI");
    RlEncoder lzw ("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI", 3);
    std::string line;
    uintmax_t total = 0;
    uint32_t lens[300] = {};
    std::ifstream data ("../data/ERR161544_1.fastq");
    int count = 0;
    if (data.is_open()) {
        time_t start = time(NULL);
        while (getline(data, line) && count++ < 10000000000) 
        {
            getline(data, line);
            getline(data, line);
            getline(data, line);
            std::shared_ptr<BitBuffer> b = lzw.encode(line);
            total += b->size();
            lens[line.length()]++;
        }
        time_t finish = time(NULL);
        uintmax_t elapsed = (uintmax_t) finish - (uintmax_t) start; 
        std::cout << "Time: " << elapsed << std::endl;
        std::cout << "Total: " << count << std::endl;
        std::cout << line << std::endl;
        std::cout << "Bytes: " << total << std::endl;
        lzw.print_occur();
    }
    for (int i = 0; i < 300; i++) {
        std::cout << i << "- " << lens[i] << std::endl;
    }
}