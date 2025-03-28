
#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <_Token.hpp>
#include <_Error_Handler.hpp>
#include <_MemoryManagement.hpp>

namespace BPE
{
    class BytePairEncoding
    {
    public:
        BytePairEncoding(size_t _t_size) : m_Table(_t_size) , m_Epoch(_t_size) {}
        ~BytePairEncoding() {}

        void Train(const std::string &&_f)
        {

            std::ifstream in(_f, std::ostream::in);

            Log(BPE::LOG_INFO, "Loading File..."); 
            
            std::stringstream Buff_Str;
            Buff_Str << in.rdbuf();
            std::string str;
            str = Buff_Str.str();

            Log(BPE::LOG_INFO, "File Size : %ld",str.length()); 
            

            Log(BPE::LOG_INFO, "Converting to Lower Case");
            // convert to all lower case english letter for easy use
            BPE::str_tolower(str);

            Log(BPE::LOG_INFO, "Converting to UF8");
            // convert to bytes of utf8
            // This Two Vector will pass Each other
            std::vector<uint32_t> FromVect = BPE::to_utf8_bytes<uint32_t>(str);
            std::vector<uint32_t> ToVect(FromVect.size());

            std::array<std::vector<uint32_t>, 2> FromToVect{FromVect, ToVect};

            Log(BPE::LOG_INFO, "Starting The Trainging Processes");
            for (size_t epoch = 0; epoch < m_Epoch; epoch++)
            {
                // Insert the Token and Calculate the Frequency
                for (size_t i = 0; i < FromToVect[epoch % 2].size() - 1; i++)
                {
                    BPE::Token T(FromToVect[epoch % 2][i], FromToVect[epoch % 2][i + 1]);
                    m_TokenMap[T]++;
                }

                // Get The Max Freq Element
                auto &max_freq = *std::max_element(m_TokenMap.begin(),
                                                   m_TokenMap.end(),
                                                   [](const std::pair<BPE::Token, uint32_t> &A, const std::pair<BPE::Token, uint32_t> &B) -> bool
                                                   {
                                                       return A.second < B.second;
                                                   });
                auto max_freq_tok = max_freq.first;
                auto max_freq_fnum = max_freq.second;

                if (max_freq_fnum <= 1)
                    break;

                m_Table.AddTokenMap(max_freq_tok);

                m_TokenMap.clear();
                max_freq_tok.reset_curr_iter_ids(m_Table.getNextStartId());

                m_Table.Encode(FromToVect[epoch % 2], FromToVect[(epoch + 1) % 2]);
            }
        }
    
        std::vector<uint32_t> Encode(const std::string &&fp)
        {
            Log(LOG_INFO,"Encoding File ... %s",fp.c_str());
            // std::vector<uint32_t>
            // m_Table.Encode();
            TODO(BytePairEncoding::Encode);
        }

        BPE::BPE_Table& getBpeTable() { return m_Table; }

    private:
        BPE::BPE_Table m_Table;
        BPE::Dictionary<BPE::Token, uint32_t> m_TokenMap;
        const size_t m_Epoch;
    };
}