#include "metadatafieldencoder.hpp"
#include "autoincrementingfieldencoder.hpp"
#include "alphanumericfieldencoder.hpp"
#include "constantalphanumericfieldencoder.hpp"
#include "numericfieldencoder.hpp"


int main(void) 
{
    /*std::string metadata = "@ERR161544.9 B81CBVABXX:5:1:1310:2174#CNNNNNNN/1";
    std::array<int, tokens> tokens = tokenize(metadata);
    for (int i = 0; i < tokens; i++)
        cout << tokens[i] << "\n";*/
    std::shared_ptr<BitBuffer> b(new BitBuffer);
    //NumericFieldEncoder n(b, 8, false);
    //AutoIncrementingFieldEncoder n(b, 1);
    ConstantAlphanumericFieldEncoder n(b, "testing1 2 3 ");
    n.encode("1");
    b->print();
    n.encode("2");
    b->print();
    n.encode("3");
    b->print();
    n.encode("4");
    b->print();
    n.encode("127");
    b->print();
    std::ostream oss;
    for (int i = 0; i < 12; i++)
        n.decode(oss);
    std::cout << oss.str() << "hello\n";
    
}