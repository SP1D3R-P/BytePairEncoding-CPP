
# define LOG_LEVEL 0

#include <BytePairEncoding.hpp>
# include <fstream>


int main()
{

    BPE::BytePairEncoding bpe(10);
    bpe.Train("../TestData.en");

    std::ofstream f("../Output.txt");

    Log(BPE::LOG_INFO,"Logging Out The Tokens");
    bpe.getBpeTable()
            .PrintToken(0,INT32_MAX,f);

    return 0;
}