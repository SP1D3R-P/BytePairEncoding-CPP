
# define LOG_LEVEL 0
# define OPTIMIZED
#include <BytePairEncoding.hpp>
# include <fstream>


int main()
{

    BPE::BytePairEncoding bpe(5000);
    bpe.Train("../Corpus/TestData.en");

    std::ofstream f("../Output/Output.txt");

    Log(BPE::LOG_INFO,"Logging Out The Tokens");
    bpe.getBpeTable()
            .PrintToken(0,INT32_MAX,f);

    return 0;
}