
# include <BPE_Common.hpp>

/**
 * [x,y] -> one token
 * freq [x,y]
 * max of the freq
 * update the vocab <- new token
 */


namespace BPE
{

    class BPE_Table;

    /*******************************************************************************************************************************/
    class Token : public DictionaryKeyInterface<Token>
    {

        /**
         * Requres [ a hash function ]
         * Compare Function if both are same or not
         * new token range (256 -INT32_MAX )
         */

    public:
        /**
         *  [x = l.key , y = r.key]
         */
        Token(uint32_t x, uint32_t y)
            : m_data{x, y}
        {}
        Token(const Token &B)
            : m_data{B.m_data[0], B.m_data[1]}, m_id(B.m_id)
        {}

        Token() {} // for now 
        /**
        *   Required Operator for the Dictionary
        */
        virtual bool operator==(const Token &B) const final { return m_data[0] == B.m_data[0] && m_data[0] == B.m_data[0]; }
        virtual Token &operator=(const Token &B)
        {
            memcpy(m_data, B.m_data, sizeof m_data);
            m_id = B.m_id;
            return *this;
        }
        virtual Token &operator=(const Token &&B) = delete;
        /**
        * Returns Id of the Token if set else just 0
        */
        uint32_t getId() const { return m_id; }
        /**
        *   Returns the [l.key , r.key ]
        */
        const std::pair<uint32_t, uint32_t> getData() const { return {m_data[0], m_data[1]}; }
        /**
        *   Returns the hash of the Token using the [l.key & r.key]
        */
        static uint64_t __hash__(uint32_t data0, uint32_t data1) { return ((uint64_t)data0 << 0x20) | data1; }

    private:
        virtual uint64_t __hash__() final { return ((uint64_t)m_data[0] << 0x20) | m_data[1]; }

    private:
        uint32_t m_data[2];
        uint32_t m_id;

        friend class BPE_Table;
    };

    /*******************************************************************************************************************************/
    /**
     *
     *  BPE_Table ::
     *
     */
    class BPE_Table
    {
    public:
        BPE_Table(size_t tableSize) : m_table_size(tableSize) , m_tokens(0x100) // book first 256 tokesn 
        { 
            m_tokens.reserve(tableSize);
            
        }
        ~BPE_Table() {}

        const Token &addToken(Token &tok)
        {
            // Log(LOG_WARN,"Curr = %zu , Max %zu bool %d",m_CurrId,m_table_size,m_CurrId >= m_table_size);
            assert(m_tokens.size() < m_table_size && "Trying To Fit More than the allocated Size.");
            tok.m_id = m_tokens.size(); // set the token id

            // As the New Tok Gen starts from 256
            m_token_map[tok.__hash__()] = m_tokens.size(); // update the map

            m_tokens.push_back(tok); // save the values

            return m_tokens[m_tokens.size() - 1];
        }// addToken
        const Token& addBasicToken(Token& tok)
        {

        }// addBasicToken
        /**
         * Encode 
         *  - Encode the from vector to to Vect
         */
        void encode(const std::vector<uint32_t> &from, std::vector<uint32_t> &to)
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
                auto Tok_It = m_token_map.find(hash);

                if (Tok_It == m_token_map.end()) // The Token Does not Exists
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

        

        void decode(const std::vector<uint32_t> &from, std::vector<uint32_t> &to)
        {
            TODO(BPE_TABLE::decode);
        }

        // Prints All the Token in range [start , end ]
        void printToken(uint32_t start = 0, uint32_t end = INT32_MAX, std::ostream &buff = std::cout)
        {
            std::string Tok_str;
            for (uint32_t i = 0; i < m_tokens.size(); i++)
            {
                auto &Tok = m_tokens[i];
                char Buff[1024] = {0};
                sprintf(Buff, "[%zu] => ", i);

                Tok_str.append(Buff);

                evalToken(Tok, Tok_str);
                buff << Tok_str << "\n";
                Tok_str.clear();
            }
        }

    private:
        void evalToken(const Token &Tok, std::string &Res)
        {
            auto ch1 = Tok.m_data[0];
            auto ch2 = Tok.m_data[1];

            /**
             * if ch1 or ch2 > 0x80 then they are not part of the inital ascii
             * to find them go to token ch1 or ch2 i.e m_tokens[ch1 or ch2 - 0x100 ] as they are stored in the vector index
             */

            if (ch1 > 0xff)
                // Evaluate Left Token
                evalToken(m_tokens[ch1], Res);
            else
                // Append the Text
                Res.append(1, (char)ch1);

            if (ch2 > 0xff)
                // Evaluate Right Token
                evalToken(m_tokens[ch2], Res);
            else
                // Append the Text
                Res.append(1, (char)ch2);
        }

    private:
        std::unordered_map<uint64_t /*id*/, size_t /*index to the vector*/> m_token_map;
        std::vector<Token> m_tokens;
        const size_t m_table_size;

        friend class PyBytePairEncoding;
    };
}