
#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <_Token.hpp>
#include <_Error_Handler.hpp>
#include <_MemoryManagement.hpp>

namespace BPE
{
    class BytePairEncoding
    {
    public:
        BytePairEncoding(size_t VocabSize = 200) : m_Vocab(VocabSize), m_Epoch(VocabSize) {}
        ~BytePairEncoding()
        {
            delete m_ToVect;
            delete m_FromVect;
        }

        BytePairEncoding &Compile(std::string &&_f, bool ConvertLower = true)
        {
            try
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

                m_FileNames.insert(_f);

                m_FromVect = BPE::to_utf8_bytes<uint32_t>(str);
                m_ToVect = new std::vector<uint32_t>(m_FromVect->size());
            }
            catch (...)
            {
                Log(LOG_ERROR, "Can't Read File %s, Something went wrong ", _f.c_str());
            }

            return *this;
        }

        BytePairEncoding &Train(
            const size_t LogFreq = 200)
        {
            if (m_FileNames.size() < 1)
            {
                Log(LOG_ERROR, "Something went Wrong or No File is Loaded ");
                return *this;
            }
#if LOG_LEVEL < 1
            char TimmerBufferString[128] = {0};
            sprintf(TimmerBufferString, "Training for %zu Epoches ", LogFreq);
#endif // LOG_LEVEL < 1
            std::array<std::vector<uint32_t>, 2> FromToVect{*m_FromVect, *m_ToVect};
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
#if LOG_LEVEL < 1
                auto ShouldLog = epoch % LogFreq == 0;
                TimeIt Timmer(TimmerBufferString, ShouldLog);
#endif                                                // LOG_LEVEL < 1
                int file_id = m_FileNames.size() - 1; // Last File is already loaded

#ifndef OPTIMIZED // un optimized Algo
                do{// insert all the tokens from all the file 
                    
                    // Insert the Token and Calculate the Frequency
                    for (size_t i = 0; i < FromToVect[epoch % 2].size() - 1; i++)
                    {
                        BPE::Token T(FromToVect[epoch % 2][i], FromToVect[epoch % 2][i + 1]);
                        m_TokenMap[T]++;
                    }
                    --file_id;
                    /**
                     * Load the all the From Files only 
                     */
                    if(file_id > 0)
                    {
                        // load other file 
                    }
                } while( file_id > 0 );
                file_id = m_FileNames.size() - 1; // reset the file id count 
#endif // UNOPTIMIZED
                /**
                 * Common Portion
                 */

                // Get The Max Freq Element
                auto &max_freq = *std::max_element(m_TokenMap.begin(),
                                                   m_TokenMap.end(),
                                                   [](const std::pair<BPE::Token, uint32_t> &A, const std::pair<BPE::Token, uint32_t> &B) -> bool
                                                   {
                                                       return A.second < B.second;
                                                   });
                auto max_freq_tok = max_freq.first;
                auto max_freq_fnum = max_freq.second;

                auto &AddedTok = m_Vocab.AddTokenMap(max_freq_tok); // insert the max token

                if (max_freq_fnum < 2)
                {
                    Log(LOG_WARN, "The Min Freq Has been Reached Terminating Training at %zu Epoch", epoch);
                    return *this;
                }
                do
                {
#ifndef OPTIMIZED

                    m_TokenMap.clear();

                    m_Vocab.Encode(FromToVect[epoch % 2], FromToVect[(epoch + 1) % 2]);
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

                    m_TokenMap.erase(max_freq_tok); // remove the max token from the temp Token Map

                    auto const &from = FromToVect[epoch % 2];
                    auto &to = FromToVect[(epoch + 1) % 2];

                    to.clear();

                    size_t i; // counter

                    // Encode Inplace
                    for (i = 0; i < from.size() - 1; i++)
                    {
                        auto curr_word = from[i];
                        auto next_word = from[i + 1];

                        // calculating hash
                        auto hash = Token::__hash__(curr_word, next_word);

                        // find the Token
                        auto Tok_It = m_Vocab.m_TokenMap.find(hash);

                        if (Tok_It == m_Vocab.m_TokenMap.end()) // The Token Does not Present
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
                                auto &nextToken = m_Vocab.m_Tokens[Tok_idx - 0x100];

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

#if LOG_LEVEL < 1
                    if (ShouldLog)
                    {
                        printf("\n\n");
                        printf("--------------------------------------------------------------\n");
                        Log(LOG_INFO, "Epoch :: %zu tem_Tokens : %zu", epoch, m_TokenMap.size());
                        Log(LOG_INFO, "Context Len : %zu ", FromToVect[(epoch + 1) % 2].size());
                        printf("--------------------------------------------------------------\n");
                    }
#endif // LOG_LEVEL < 1
                    /**
                     * Load Different File
                     * and save the To [ file naming will be like acc_name_{epoch%2}]
                     */
                    --file_id;
                    if(file_id > 0)
                    {
                        // load other file 
                    }
                } while (file_id > 0);

            } // main for loop [ epoches ]
            
            // Remove the Processed Files From the Folder 
            m_FileNames.clear();
            return *this;
        }

        std::vector<uint32_t> Encode(const std::string &&fp)
        {
            Log(LOG_INFO, "Encoding File ... %s", fp.c_str());
            TODO(BytePairEncoding::Encode);
        }

        std::vector<uint32_t> Decode(const std::string &&fp)
        {
            Log(LOG_INFO, "Decoding File ... %s", fp.c_str());
            TODO(BytePairEncoding::Decode);
        }

        void Save()
        {
            TODO(BytePairEncoding::Save);
        }

        BPE::BPE_Table &getBpeTable() { return m_Vocab; }

    private:
        BPE::BPE_Table m_Vocab;
        BPE::Dictionary<BPE::Token, uint32_t> m_TokenMap;
        const size_t m_Epoch;
        std::vector<uint32_t> *m_FromVect;
        std::vector<uint32_t> *m_ToVect;

        std::unordered_set<std::string> m_FileNames;
    };
}