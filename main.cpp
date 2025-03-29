
#define LOG_LEVEL 0
#define OPTIMIZED
#include <BytePairEncoding.hpp>
#include <fstream>

int main()
{

    BPE::BytePairEncoding bpe(100);
    bpe
        .Compile("../Corpus/Smalltext.en")
        .Compile("../Corpus/Smalltext2.en")
        .Train(10);

    std::ofstream f("../Output/Output.txt");

    Log(BPE::LOG_INFO, "Logging Out The Tokens");
    bpe.getBpeTable()
        .PrintToken(0, INT32_MAX, f);

    return 0;
}