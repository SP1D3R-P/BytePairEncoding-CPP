
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
        BytePairEncoding(size_t _t_size = 200) : m_Table(_t_size), m_Epoch(_t_size) {}
        ~BytePairEncoding() {}

        void Train(const std::string &&_f)
        {
            std::ifstream in(_f, std::ostream::in);

            Log(BPE::LOG_INFO, "Loading File...");

            std::stringstream Buff_Str;
            Buff_Str << in.rdbuf();
            std::string str;
            str = Buff_Str.str();

            Log(BPE::LOG_INFO, "File Size : %ld", str.length());

            Log(BPE::LOG_INFO, "Converting to Lower Case");
            // convert to all lower case english letter for easy use
            BPE::str_tolower(str);

            Log(BPE::LOG_INFO, "Converting to UF8");
            // convert to bytes of utf8
            // This Two Vector will pass Each other
            std::vector<uint32_t> FromVect = BPE::to_utf8_bytes<uint32_t>(str);
            std::vector<uint32_t> ToVect(FromVect.size());

            // Above part is preprocessing need to be
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            std::array<std::vector<uint32_t>, 2> FromToVect{FromVect, ToVect};
#ifdef OPTIMIZED
            /**
             * Calculate the Frequence Table  for the first time
             */

            // Insert the Token and Calculate the Frequency
            for (size_t i = 0; i < FromToVect[0].size() - 1; i++)
            {
                BPE::Token T(FromToVect[0][i], FromToVect[0][i + 1]);
                m_TokenMap[T]++;
            }

#endif // OPTIMIZED

            Log(BPE::LOG_INFO, "Starting The Trainging Processes");
            for (size_t epoch = 0; epoch < m_Epoch; epoch++)
            {
#ifndef OPTIMIZED // un optimized Algo

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

                m_Table.Encode(FromToVect[epoch % 2], FromToVect[(epoch + 1) % 2]);
#endif
#ifdef OPTIMIZED // Use Optimized Algorithm

                /**
                 *
                 * Implementing inplace
                 *  __
                 * abcbd
                 *
                 * let max token be bc
                 *
                 * aX Xb d
                 *
                 * remove [b,c] token from the map
                 * now add new token aX , Xb
                 */

                // clear the memory

                // Get The Max Freq Element
                auto &max_freq = *std::max_element(m_TokenMap.begin(),
                                                   m_TokenMap.end(),
                                                   [](const std::pair<BPE::Token, uint32_t> &A, const std::pair<BPE::Token, uint32_t> &B) -> bool
                                                   {
                                                       return A.second < B.second;
                                                   });
                auto max_freq_tok = max_freq.first;
                auto max_freq_fnum = max_freq.second;

                if (max_freq_fnum < 2)
                {
                    Log(LOG_WARN,"The Min Freq Has been Reached Terminating Training at %zu Epoch",epoch);
                    break;
                }

                auto &AddedTok = m_Table.AddTokenMap(max_freq_tok); // insert the max token
                m_TokenMap.erase(max_freq_tok);                     // remove the max token from the temp Token Map

                auto const &from = FromToVect[epoch % 2];
                auto &to = FromToVect[(epoch + 1) % 2];

                to.clear();

                if (epoch % 100 == 0)
                {

                    Log(LOG_INFO, "Epoch :: %zu Tokens : %zu", epoch, m_TokenMap.size());
                    Log(LOG_INFO, "Max :: [%zu , %zu ]", max_freq_tok.getData().first, max_freq_tok.getData().second);
                    Log(LOG_INFO, "Context Len : %zu ", from.size());

                    // for (auto k : from)
                    // {
                    //     printf("[ %zu ]",k);
                    // }
                    // printf("\n\n");
                    

                }
                size_t i; // counter

                // Encode Inplace
                for (i = 0; i < from.size() - 1; i++)
                {
                    auto curr_word = from[i];
                    auto next_word = from[i + 1];

                    // calculating hash
                    auto hash = Token::__hash__(curr_word, next_word);

                    // find the Token
                    auto Tok_It = m_Table.m_TokenMap.find(hash);

                    if (Tok_It == m_Table.m_TokenMap.end()) // The Token Does not Present
                    {
                        to.push_back(curr_word);
                        continue;
                    }

                    const auto [Tok_hash, Tok_idx] = *Tok_It;

                    // check this token is the curr max token
                    if (Tok_idx == AddedTok.getId())
                    {
                        auto gen_TokenLen = to.size();

                        if (gen_TokenLen > 0) // is there any token before
                        {
                            auto &prev_gen_word = to[gen_TokenLen - 1];
                            auto &nextToken = m_Table.m_Tokens[Tok_idx - 0x100];

                            /**
                             * For ababd
                             * max freq ab
                             * X <-
                             * Xa freq ++
                             * XX <-
                             * Xa freq -- and Xd freq ++
                             * d <-
                             * done
                             */

                            auto prevTokenPair = Token(prev_gen_word, curr_word);
                            // New Token
                            Token nT(prev_gen_word, Tok_idx);

                            m_TokenMap[prevTokenPair]--;
                            m_TokenMap[nT]++; // Increase the Count of the Freq new type
                        }

                        // Add The Max Pair
                        to.push_back(Tok_idx);

                        // if there is Next token Then do
                        if ((i + 2) < from.size())
                        {
                            Token PrevFutureToken(next_word, from[i + 2]);
                            Token CurrentFutureToken(Tok_idx, from[i + 2]);

                            m_TokenMap[PrevFutureToken]--;

                            m_TokenMap[CurrentFutureToken]++;
                        }
                    }
                    else
                    {
                        // We Know Tok Id = Tok_idx
                        to.push_back(Tok_idx); // Unreachable
                    }
                    // jump the next word
                    i += 1;
                }

                if (i < from.size())
                    to.push_back(from[i]);

#endif // OPTIMIZED
            } // main for loop [ epoches ]

            // Recalculate the Max
        }

        std::vector<uint32_t> Encode(const std::string &&fp)
        {
            Log(LOG_INFO, "Encoding File ... %s", fp.c_str());
            // std::vector<uint32_t>
            // m_Table.Encode();
            TODO(BytePairEncoding::Encode);
        }

        BPE::BPE_Table &getBpeTable() { return m_Table; }

    private:
        BPE::BPE_Table m_Table;
        BPE::Dictionary<BPE::Token, uint32_t> m_TokenMap;
        const size_t m_Epoch;
    };
}