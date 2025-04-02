
// #define LOG_LEVEL 0
// #define OPTIMIZED
// #include <BytePairEncoding.hpp>

// #include <common.hpp>
// int main(int argc, char **argv)
// {

//     // BPE::BytePairEncoding bpe(1000);
//     // bpe
//     //     .Compile("../Corpus/TestData.en")
//     //     .Train(100);

//     // std::ofstream f("../Output/Output.txt");

//     // Log(BPE::LOG_INFO, "Logging Out The Tokens");
//     // bpe.getBpeTable()
//     //     .PrintToken(0, INT32_MAX, f);

//     // real	0m6.787s
//     // user	0m5.603s
//     // sys	0m0.449s

//     std::string word(argv[1]);

//     boost::regex re(GPT4_PATTERN);


    

//     auto it_begin = boost::sregex_iterator(word.begin(), word.end(), re);
//     auto it_end = boost::sregex_iterator();

//     for (auto i = it_begin; i != it_end; ++i)
//     {
//         boost::smatch match = *i;
//         std::string match_str = match.str();
//         std::cout << match_str << std::endl;
//     }

//     return 0;
// }


// # include <pybind11/pybind11.h>
// # include <BytePairEncoding.hpp>

// int main() {
    
//     std::string text = "Hello, 234world! How are you?";
//     boost::regex re(GPT4_PATTERN);

//     boost::sregex_iterator iter(text.begin(), text.end(), re);
//     boost::sregex_iterator end;

//     std::vector<std::string> tokens;
//     for (; iter != end; ++iter) {
//         if (iter->str().empty()) {
//             continue;
//         }
//         tokens.push_back(iter->str());
//     }

//     for (const auto& token : tokens) {
//         std::cout << token << std::endl;
//     }

//     return 0;
// }