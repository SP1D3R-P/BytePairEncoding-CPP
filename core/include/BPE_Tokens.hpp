
# include <BPE_Common.hpp>
# include <sstream>
/**
 * [x,y] -> one token
 * freq [x,y]
 * max of the freq
 * update the vocab <- new token
 */


namespace BPE
{

    class BPE_Table;
    namespace py = pybind11;

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
        virtual bool operator==(const Token &B) const final { return m_data[0] == B.m_data[0] && m_data[1] == B.m_data[1]; }
        virtual bool operator==(const std::pair<uint32_t,uint32_t> &B) const final { return m_data[0] == B.first && m_data[1] == B.second;}
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
            virtual uint64_t __hash__() const final { return ((uint64_t)m_data[0] << 0x20) | m_data[1]; }

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
            m_basic_tok_count = 0 ;
            m_basic_tok_count_end = false;
            assert(tableSize > 0x100 && "Vocab size Must be Greater than 256 ");
            m_tokens.reserve(tableSize);
        }
        ~BPE_Table() {}

        PYBIND11_NOINLINE virtual const Token& addToken(Token &tok) final 
        {
            assert(m_tokens.size() < m_table_size && "Trying To Fit More than the allocated Size.");
            tok.m_id = m_tokens.size(); // set the token id

            // As the New Tok Gen starts from 256
            m_token_map[tok.__hash__()] = m_tokens.size(); // update the map

            m_tokens.push_back(tok); // save the values

            return m_tokens[m_tokens.size() - 1];
        }// addToken

        PYBIND11_NOINLINE virtual const Token& addBasicToken(char x) final 
        {
            assert(!m_basic_tok_count_end && "Can't Insert an basic token after an compund token is inserted.");
            Token basicTok(0,x);
            return m_basic_tok_count > 0x100 ? 
                    addToken(basicTok) :
                    ( 
                        m_token_map[basicTok.__hash__()] = m_basic_tok_count + 1,
                        m_tokens[m_basic_tok_count++] = basicTok
                    );
        }// addBasicToken
        
        PYBIND11_NOINLINE static BPE_Table loadFromBinary(void *binary, uint64_t size) __THROW
        {

        } 

        PYBIND11_NOINLINE virtual std::pair<void*,size_t> dumptoBinary() final 
        {

        } 

        PYBIND11_NOINLINE virtual size_t encode(std::vector<uint32_t> &utf8_stream ) const final 
        {
            size_t final_size = utf8_stream.size();
            for(auto &tok : m_tokens )
            {
                size_t curr_iter_size = 0;
                size_t index;
                auto [t1,t2] = tok.getData();
                for (index = 0 ; index < final_size-1  ; ++index)
                {
                    if(utf8_stream[index] == t1 &&
                        utf8_stream[index+1] == t2
                    ){
                        utf8_stream[curr_iter_size++] = tok.getId();
                        ++index;
                    } else {
                        utf8_stream[curr_iter_size++] = utf8_stream[index];
                    }
                }
                if(index != final_size)
                {
                    utf8_stream[curr_iter_size++] = utf8_stream[index];
                }
                final_size = curr_iter_size;
            }

            return final_size;
        } // encode 

        PYBIND11_NOINLINE virtual py::str decode_token(const std::vector<uint32_t> &encoded_vec) const final
        {
            std::string result_string;
            result_string.reserve(encoded_vec.size());
            for(auto &tok : encoded_vec)
            {
                if(tok >= m_table_size || tok >= m_tokens.size() ) {
                    std::ostringstream buffer_string;
                    buffer_string << "Unknown Token Passed: " << tok << ".\n";
                    std::string error = buffer_string.str();
                    throw py::value_error(error);
                } else if( tok < 0xff ) /*for now only*/{
                    result_string.append(1,(char)tok);
                } else {
                    auto token = m_tokens[tok];
                    evalToken(token,result_string);
                }
            }
            return py::str{result_string};
        } // decode

        PYBIND11_NOINLINE virtual 
            std::pair<bool,uint64_t>
            isTokenPresent(const uint64_t &hash)
        {
            auto value = m_token_map.find(hash);
            bool Present = m_token_map.end() == value ; 
            return {
                !Present , 
                Present ? 
                    INT64_MAX :
                    (*value).second 
            };
            
        }

        // Prints All the Token in range [start , end ]
        PYBIND11_NOINLINE virtual void printToken(std::ostream &buff = std::cout) const final 
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
        PYBIND11_NOINLINE virtual void evalToken(const Token &Tok, std::string &Res) const final
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
        size_t m_basic_tok_count;
        bool m_basic_tok_count_end;
    };
}
