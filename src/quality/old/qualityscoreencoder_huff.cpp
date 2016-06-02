#include "bitbuffer.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

class QualityScoreEncoder {
  private:
  static const int map[][2];
    std::shared_ptr<BitBuffer> b;
  public:
    QualityScoreEncoder(std::shared_ptr<BitBuffer>);
    void encode(std::string);
};

const int QualityScoreEncoder::map[][2] = {{5309974,23},{5309975,23},{5309976,23},{5309977,23},{5309978,23},{5309979,23},{5309980,23},{5309981,23},{41480,16},{41485,16},{20741,15},{5184,13},{6406,13},{6407,13},{7649,13},{3292,12},{3746,12},{1297,11},{5325,13},{7397,13},{3202,12},{3293,12},{3747,12},{1333,11},{1332,11},{1600,11},{1647,11},{653,10},{667,10},{921,10},{953,10},{327,9},{410,9},{469,9},{479,9},{204,8},{235,8},{82,7},{114,7},{47,6},{55,6},{22,5},{7,4},{4,3},{0,3},{48,6},{1,3},{2,3},{6,4},{21,5},{54,6},{46,6},{103,7},{80,7},{232,8},{199,8},{477,9},{461,9},{397,9},{325,9},{937,10},{801,10},{664,10},{1848,11},{1872,11},{1330,11},{3825,12},{3699,12},{2663,12},{2593,12},{7648,13},{5324,13},{14792,14},{14793,14},{20743,15},{82969,17},{41481,16},{165937,18},{5309982,23},{5309983,23},{2654976,22},{2654977,22},{2654978,22},{2654979,22},{2654980,22},{2654981,22},{2654982,22},{2654983,22},{2654984,22},{2654985,22},{2654986,22},{15,4},{26,5},{56,6},{118,7},{98,7},{233,8},{201,8},{167,8},{463,9},{401,9},{396,9},{957,10},{952,10},{925,10},{920,10},{822,10},{649,10},{652,10},{1913,11},{101,7}};

QualityScoreEncoder::QualityScoreEncoder(std::shared_ptr<BitBuffer> buffer) : b(buffer)
{
    
}

void 
QualityScoreEncoder::encode(std::string qs)
{
    char prev = 'H';
    int consec_zero = 0;
    const int consec_max = 20;
    const int consec_offset = 90;
    const int normal_offset = 45;
    const int *mapping;
    int bits = 0;
    for (auto it = qs.begin(); it != --qs.end(); it++)
    {
        char curr = *it;
        int delta = curr - prev;
        if (delta == 0)
        {
            if (consec_zero++ == consec_max)
            {
                mapping = map[consec_offset + consec_max];
                b->write(mapping[0], mapping[1]);   
                bits += mapping[1];
                //std::cout << "Consec zeros: " << consec_zero << std::endl;
                consec_zero = 0;
            }
        } 
        else
        {
            if (consec_zero > 0)
            {
                mapping = map[consec_offset + consec_zero];
                b->write(mapping[0], mapping[1]);
                bits += mapping[1];
                //std::cout << "Consec zeros: " << consec_zero << std::endl;
                consec_zero = 0;
            }
            mapping = map[normal_offset + delta];
            b->write(mapping[0], mapping[1]);
            bits += mapping[1];
            //std::cout << "Change: " << delta << std::endl;
            prev = curr;
        }
    }
    b->write(map[normal_offset][0], map[normal_offset][1]);
    bits += map[normal_offset][1];
    static int entry = 1;
    //std::cout << "Entry " << entry++ << " -> " << bits << " bits\n";
}

int main(int argc, char ** argv)
{
    if (argc < 2)
        return -1;
    int entries = 10000;
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    QualityScoreEncoder enc(b);
    std::ifstream file(argv[1], std::ifstream::in);
    std::string metadata;
    for (int rem = 0; rem < entries && std::getline(file, metadata); rem++)
    {
        std::getline(file, metadata);
        std::getline(file, metadata);
        std::getline(file, metadata);
        enc.encode(metadata);        
    }
    std::cout << "File size: " << b->size() << std::endl;
}