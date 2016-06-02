#include <string>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>

/*class MetadataEncoder {
    public:
        std::array<int, tokens> tokenize(string);
    private:
        ofstream file;
        int tokens_no;
};*/

std::string meta_sep = "";
int meta_const = 0;
int meta_var = 0;
std::vector<int> const_pos;

std::array<std::string, 10> init(std::string metadata) {
    const std::string sep = "@. :#/";
    std::array<std::string, 10> tokens;
    int index = 0;
    int pos = 0;
    std::string curr = "";
    int add = 0;
    for ( std::string::iterator it=++metadata.begin(); it!=metadata.end(); ++it) {
        if (sep.find(*it) == std::string::npos) {
            if (*it > '9' || *it < '0') {
                add = 1;
            }
            curr.append(1, *it);
        } else {
            if (add) {
                tokens[index++] = curr;
                meta_const++;
                const_pos.push_back(pos);
            } else { meta_var++; }
            pos++;
            add = 0;
            curr = "";
            meta_sep.append(1, *it);
        }
    }
    if (add && curr.empty()) {
        tokens[index++] = curr;
        meta_const++;
    } else { meta_var++; }
    tokens[index++] = curr;
    return tokens;   
}

std::array<int, 10> tokenize(std::string metadata) {
    const std::string sep = "@. :#/";
    std::array<int, 10> tokens;
    int index = 0;
    int curr = 0;
    for ( std::string::iterator it=++metadata.begin(); it!=metadata.end(); ++it) {
        if (sep.find(*it) == std::string::npos) {
            if (*it > '9' || *it < '0') {
                curr = 0;
                while (sep.find(*it) == std::string::npos) { it++; }
            } else { 
                curr = curr * 10 + (*it - '0');
            }
        } else {
            tokens[index++] = curr;
            curr = 0;
        }
    }
    tokens[index++] = curr;
    return tokens;
}

int main(void) 
{
    std::string metadata = "@ERR161544.9 B81CBVABXX:5:1:1310:2174#CNNNNNNN/1";
    std::array<std::string, 10> s_tokens = init(metadata);
    std::cout << meta_sep << "\n";
    for (int i = 0; i < meta_const; i++)
        std::cout << s_tokens[i] << "\n";   
    std::array<int, 10> tokens = tokenize(metadata);
    for (int i = 0; i < meta_var; i++)
        std::cout << tokens[i] << "\n";
    for (int i = 0; i < meta_const; i++)
        std::cout << const_pos.at(i) << "\n";
}