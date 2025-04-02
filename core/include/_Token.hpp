


#include <_Dictionary_InterFace.hpp>
#include <_Error_Handler.hpp>

# include <common.hpp>

/**
 * [x,y] -> one token
 * freq [x,y]
 * max of the freq
 * update the vocab <- new token
 */


namespace BPE
{

    using UTF8 = unsigned char;
    using Bytes = unsigned char;
    class BPE_Table;

    template <typename T = uint32_t>
    static bool to_utf8_bytes(const std::string &input, std::vector<T> &Ret_vec)
    {
        UErrorCode status = U_ZERO_ERROR;

        // Convert input string to UnicodeString
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(input);

        // Check for errors after conversion
        if (U_FAILURE(status))
        {
            std::cerr << "Conversion to UnicodeString failed: " << u_errorName(status) << std::endl;
            return false;
        }
        try
        {
            // Convert UnicodeString to UTF-8
            std::string utf8_str;
            unicode_str.toUTF8String(utf8_str);

            Ret_vec.clear();

            if (Ret_vec.capacity() < utf8_str.length())
                Ret_vec.reserve(utf8_str.length());

            for (size_t i = 0; i < utf8_str.length(); i++)
                Ret_vec.push_back((T)(unsigned char)utf8_str[i]);
                }
        catch (const std::exception &e)
        {
            std::cerr << "Error during UTF-8 conversion: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    static void str_tolower(std::string &_s)
    {
        for (auto &i : _s)
            i = tolower(i);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *
     *  Token ::
     *
     */
    class Token : public DictionaryKeyInterface<Token>
    {

        /**
         * Requres [ a hash function ]
         * Compare Function if both are same or not
         * new token range (256 -INT32_MAX )
         */

    public:
        /**
         *  It's user Duty to not to create new && maintain it
         */
        Token(uint32_t x, uint32_t y)
            : m_data{x, y}
        {
        }
        Token(const Token &B)
            : m_data{B.m_data[0], B.m_data[1]},
              m_id(B.m_id)
        {
        }

        virtual bool operator==(const Token &B) const final { return m_data[0] == B.m_data[0] && m_data[0] == B.m_data[0]; }

        virtual Token &operator=(const Token &B)
        {
            memcpy(m_data, B.m_data, sizeof m_data);
            m_id = B.m_id;
            return *this;
        }
        virtual Token &operator=(const Token &&B) = delete;

        uint32_t getId() const { return m_id; }

        const std::pair<uint32_t, uint32_t> getData() const { return {m_data[0], m_data[1]}; }
        static uint64_t __hash__(uint32_t data0, uint32_t data1) { return ((uint64_t)data0 << 0x20) | data1; }

    private:
        virtual uint64_t __hash__() final { return ((uint64_t)m_data[0] << 0x20) | m_data[1]; }

    private:
        uint32_t m_data[2];
        uint32_t m_id;

        friend class BPE_Table;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *
     *  BPE_Table ::
     *
     */
    class BPE_Table
    {
    public:
        BPE_Table(size_t TableSize) : m_TableSize(TableSize) { m_Tokens.reserve(TableSize); }
        ~BPE_Table() {}

        const Token &AddTokenMap(Token &T)
        {
            // Log(LOG_WARN,"Curr = %zu , Max %zu bool %d",m_CurrId,m_TableSize,m_CurrId >= m_TableSize);
            assert(m_CurrId - 0x100 < m_TableSize && "Trying To Fit More than the allocated Size.");
            T.m_id = m_CurrId++; // set the token id

            // As the New Tok Gen starts from 256
            m_TokenMap[T.__hash__()] = m_Tokens.size() + 0x100; // update the map

            m_Tokens.push_back(T); // save the values

            return m_Tokens[m_Tokens.size() - 1];
        }

        void Encode(const std::vector<uint32_t> &from, std::vector<uint32_t> &to)
        {
            // clear the memory
            to.clear();

            size_t i;
            for (i = 0; i < from.size() - 1; i++)
            {
                auto curr_word = from[i];
                auto next_word = from[i + 1];

                // calculating hash
                auto hash = Token::__hash__(curr_word, next_word);

                // find the Token
                auto Tok_It = m_TokenMap.find(hash);

                if (Tok_It == m_TokenMap.end()) // The Token Does not Exists
                {
                    to.push_back(curr_word);
                    continue;
                }

                auto [Tok_hash, Tok_idx] = *Tok_It;

                // We Know Tok Id = Tok_idx
                to.push_back(Tok_idx);

                // jump the next word
                i += 1;
            }

            if (i < from.size())
                to.push_back(from[i]);
        }

        void Decode(const std::vector<uint32_t> &from, std::vector<uint32_t> &to)
        {
            TODO(BPE_TABLE::Decode);
        }

        // Prints All the Token in range [start , end ]
        void PrintToken(uint32_t start = 0, uint32_t end = INT32_MAX, std::ostream &buff = std::cout)
        {
            std::string Tok_str;
            for (uint32_t i = 0; i < m_Tokens.size(); i++)
            {
                auto &Tok = m_Tokens[i];
                char Buff[1024] = {0};
                sprintf(Buff, "[%zu] => ", i);

                Tok_str.append(Buff);

                EvalToken(Tok, Tok_str);
                buff << Tok_str << "\n";
                Tok_str.clear();
            }
        }

    private:
        void EvalToken(const Token &Tok, std::string &Res)
        {
            auto ch1 = Tok.m_data[0];
            auto ch2 = Tok.m_data[1];

            /**
             * if ch1 or ch2 > 0xff then they are not part of the inital ascii
             * to find them go to token ch1 or ch2 i.e m_Tokens[ch1 or ch2 - 0x100 ] as they are stored in the vector index
             */

            if (ch1 > 0xff)
                // Evaluate Left Token
                EvalToken(m_Tokens[ch1], Res);
            else
                // Append the Text
                Res.append(1, (char)ch1);

            if (ch2 > 0xff)
                // Evaluate Right Token
                EvalToken(m_Tokens[ch2], Res);
            else
                // Append the Text
                Res.append(1, (char)ch2);
        }

    private:
        std::unordered_map<uint64_t /*id*/, size_t /*index to the vector*/> m_TokenMap;
        std::vector<Token> m_Tokens;
        int32_t m_CurrId = 0x0; 
        const size_t m_TableSize;

        friend class BytePairEncoding;
    };
}