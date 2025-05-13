
#include "BPE_Common.hpp"
#include <ranges>
#include <tuple>
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
        {
        }
        Token(const Token &B)
            : m_data{B.m_data[0], B.m_data[1]}, m_id(B.m_id)
        {
        }

        Token() {} // for now
        /**
         *   Required Operator for the Dictionary
         */
        virtual bool operator==(const Token &B) const final { return m_data[0] == B.m_data[0] && m_data[1] == B.m_data[1]; }
        virtual bool operator==(const std::pair<uint32_t, uint32_t> &B) const final { return m_data[0] == B.first && m_data[1] == B.second; }
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
        BPE_Table(size_t tableSize) : m_table_size(tableSize)
        {
            if (tableSize < 0x100)
                throw py::value_error("vocab size Must be Greater than 256");
                
            m_tokens.reserve(tableSize);

            for (auto r : {
                    std::views::iota(uint32_t{32},size_t {127}), // printable charecter 
                    std::views::iota(uint32_t{0},size_t{32}), // charecters not printable 1 
                    std::views::iota(uint32_t{127},size_t{0x100}) // charecters not printable 2
                }
            )
                for(auto c : r )
                    addBasicToken((char)(unsigned char)c);
            
        }

        ~BPE_Table() {}

        PYBIND11_NOINLINE virtual bool isTableFull() const final { return  m_tokens.size() >= m_table_size; } 
        
        /**
         * current number of token is the map
         */
        uint32_t totalTokens() const { return m_tokens.size(); }  
        
        PYBIND11_NOINLINE virtual void u8stream_valid_token(std::vector<uint32_t> &u8_stream) final 
        {
            for ( auto& word : u8_stream)
            {
                auto id = m_token_map.find(Token{UINT32_MAX,word}.__hash__());
                
                if(id == m_token_map.end()){
                   auto tok = addBasicToken(word);
                   word = tok.getId();
                } else {
                    word = (*id).second;
                }
            }
        }

        PYBIND11_NOINLINE virtual const size_t currSize() const final { return m_tokens.size();}
        PYBIND11_NOINLINE virtual const size_t vocabCap() const final { return m_table_size; }
        PYBIND11_NOINLINE virtual void updateVocabCap(const int new_size) final { 
            if(new_size < m_table_size )
                throw py::value_error(py::str{"The new size[{}] must be greater that old size[{}]"}.format(new_size,m_table_size));
            m_table_size = new_size;
        }  

        PYBIND11_NOINLINE virtual const Token &addToken(Token &tok) final
        {
            tok.m_id = m_tokens.size();                    // set the token id
            
            m_token_map[tok.__hash__()] = tok.m_id;        // update the map
            m_tokens.push_back(tok);                    // save the values
            
            return m_tokens[tok.m_id];
        } // addToken

        PYBIND11_NOINLINE virtual size_t encode(std::vector<uint32_t> &utf8_stream) const final
        {
            _u8stream_valid_token(utf8_stream);
            size_t final_size = utf8_stream.size();
            for (auto &tok : m_tokens)
            {
                size_t curr_iter_size = 0;
                size_t index;
                auto [t1, t2] = tok.getData();
                for (index = 0; index < final_size - 1; ++index)
                {
                    if (utf8_stream[index] == t1 &&
                        utf8_stream[index + 1] == t2)
                    {
                        utf8_stream[curr_iter_size++] = tok.getId();
                        ++index;
                    }
                    else
                    {
                        utf8_stream[curr_iter_size++] = utf8_stream[index];
                    }
                }
                if (index != final_size)
                {
                    utf8_stream[curr_iter_size++] = utf8_stream[index];
                }
                final_size = curr_iter_size;
            }

            return final_size;
        } // encode

        PYBIND11_NOINLINE virtual py::bytes decodeToken(const std::vector<uint32_t> &encoded_vec) const final
        {
            std::string result_string;
            result_string.reserve(encoded_vec.size());
            for (auto &tok : encoded_vec)
            {
                if (tok < m_tokens.size())
                    evalToken(m_tokens[tok], result_string);
                else 
                    throw py::value_error(py::str{"Unknown Token Passed: {}"}.format(py::int_{tok}));
            }
            return py::bytes{result_string};
        } // decode

        PYBIND11_NOINLINE virtual std::pair<bool, uint64_t>
        isTokenPresent(const uint64_t &hash)
        {
            auto value = m_token_map.find(hash);
            bool Present = m_token_map.end() == value;
            return {
                !Present,
                Present ? INT64_MAX : (*value).second};
        }

        PYBIND11_NOINLINE virtual py::dict toDict()
        {
            py::dict result;
            
            for (auto i : std::views::iota(size_t(0), m_tokens.size()))
            {
                const Token &t = m_tokens.at(i);
                py::bytes data ;
                
                if (t.m_data[0] == UINT32_MAX){
                    data = std::string{(char)(unsigned char)t.m_data[1]};
                    // data = data_bytes.attr("decode")();
                }
                else {
                    
                    try {
                        py::bytes prev_a = result[py::int_(t.m_data[0])];
                        py::bytes prev_b = result[py::int_(t.m_data[1])];
                        data = prev_a + prev_b;
                    } catch (py::key_error e) {
                        std::string de_crypt_word ;
                        evalToken(t,de_crypt_word);
                        data = py::bytes{de_crypt_word};
                    }
                }
                result[py::int_{i}] = data;
                
            }
            return result;
        }


    private:
        PYBIND11_NOINLINE virtual Token addBasicToken(char x) final
        {
            Token basicTok{UINT32_MAX, x};
            addToken(basicTok);
            return basicTok;
        } // addBasicToken

        PYBIND11_NOINLINE virtual void evalToken(const Token &Tok, std::string &Res) const final
        {
            auto ch1 = Tok.m_data[0];
            auto ch2 = Tok.m_data[1];

            if(!(ch1 < m_tokens.size() || ch1 == UINT32_MAX ) || !(ch2 < m_tokens.size()))
                throw std::runtime_error(py::str{"Invalid Token[{},{}] passed"}.format(py::int_{ch1},py::int_{ch2}));

            /**
             * if ch1 UINT32_MAX then it's one bit special 
             */
            if( ch1 == UINT32_MAX )
            {
                Res.append(1,(char)(unsigned char)ch2);
                return;
            }
    
            evalToken(m_tokens[ch1], Res);
            evalToken(m_tokens[ch2], Res);
        }

        PYBIND11_NOINLINE virtual void _u8stream_valid_token(std::vector<uint32_t> &u8_stream) const final 
        {
            for ( auto& word : u8_stream)
            {
                auto id = m_token_map.find(Token{UINT32_MAX,word}.__hash__());
                
                if(id == m_token_map.end())
                    throw py::value_error{py::str{"Invalid Token[{}]"}.format(py::int_{word})};
                
                word = (*id).second;
            }
        } // _u8stream_valid_token

    private:
        std::unordered_map<uint64_t /*id*/, size_t /*index to the vector*/> m_token_map;
        std::vector<Token> m_tokens;
        size_t m_table_size;
    };
}
