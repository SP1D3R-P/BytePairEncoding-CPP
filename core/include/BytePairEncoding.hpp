# ifndef _BPE_H_
# define _BPE_H_

#pragma once

# include <BPE_Tokens.hpp>

namespace BPE
{

    namespace py = pybind11;

    /*******************************************************************************************************************************/

    /**
     * First Divide The Words Into Several Cuncks to Divide using the pattern
     * Then use BPE logic to create the BPE tree
     */
    class PyBytePairEncoding
    {
        typedef std::pair<std::vector<uint32_t>, uint32_t> Word_Vect_T;
        typedef std::vector<std::pair<std::vector<uint32_t>, uint32_t>> Sent_Vect_T;

        using Freq_T = uint32_t;

    public:
        PYBIND11_NOINLINE PyBytePairEncoding(
            size_t vocabSize,
            py::str pattern = GPT4_PATTERN) __THROW
            : m_bpe_table(vocabSize),
              m_vocab_size(vocabSize),
              m_pattern(pattern)
        {
            m_regex_module = py::module_::import("regex"); /*if no regex found then throws error*/
            m_regex_pattern_compiled = m_regex_module.attr("compile")(pattern);
        }

        PYBIND11_NOINLINE virtual ~PyBytePairEncoding()
        { }

        /**
         *	compile method
         *		compile the file into a vector format for futher processing
         *	@param
         *		inputString : the string to be compiled
         *		fname : the file name
         */
        PYBIND11_NOINLINE virtual void compile(pybind11::str inputString, pybind11::str fname) noexcept(true) final
        {
            Sent_Vect_T sent_vect; // final vector of utf-8 words of the string sentence

            // tokenizes the sentence to chunks of words
            py::list tokenize_words = m_regex_pattern_compiled.attr("findall")(inputString);
            uint32_t word_count /*number of word chunks*/ = py::len(tokenize_words);
                
            // create new sentence and revese the required amount of space
            sent_vect.reserve(word_count);

            // insert the word vect to the sentence vect
            for (auto words : tokenize_words)
            {
                py::str word = words.cast<py::str>();
                std::string_view encoded_string = word.attr("encode")("UTF8").cast<std::string_view>();
                
                std::vector<uint32_t> buff(encoded_string.size());
                std::ranges::copy(encoded_string.begin(),encoded_string.end(),buff.begin());
                m_bpe_table.u8stream_valid_token(buff);

                sent_vect.push_back({buff, buff.size()});
            }
            // saving the compiled word
            m_compiled_object[fname.cast<std::string_view>()] = sent_vect;
        }

        /**
         * 	train method
         *		train the Encoder on compiled vector for given number of epoch [vocab count]
         * 	@param
         *      None
         *
         */
        PYBIND11_NOINLINE virtual void train()
        {
            for (const auto &values : m_compiled_object | std::views::values)
            {
                for (const auto &[From, size] : values)
                {
                    /**
                     * Calculate the Frequence Table  for the first time
                     */
                    // Insert the Token and Calculate the Frequency
                    for (size_t i : std::views::iota(size_t(0), size - 1))
                        m_curr_token_map[BPE::Token{From[i], From[i + 1]}]++;
                }
            }

            // parallelized task
            std::function update_encode = [this](Sent_Vect_T &vec , const BPE::Token &tok) mutable -> void
            {
                for (auto &[from, size] : vec)
                    encodeInplace(from, size, tok);
            };
            
            for(;!m_bpe_table.isTableFull();) 
            {
                /**
                 * Non-parallelizable work
                 */                

                // we know that m_curr_token_map is always greater than 1 so , it's safe to directly de-refference it 
                auto &max_freq = *std::ranges::max_element( m_curr_token_map | std::views::all , 
                    [](const auto&A , const auto &B)  {
                        return A.second < B.second;
                    }
                );

                // unwrap it max freq
                auto max_freq_tok = max_freq.first;
                auto &max_freq_fnum = max_freq.second;

                // Termination Condition
                if (max_freq_fnum < 2)
                {
                    Log(LOG_WARN, "The Min frequency Has been Reached Terminating Training at %zu Tokens", m_bpe_table.totalTokens());
                    break;
                }
                // New Added Token
                m_bpe_table.addToken(max_freq_tok); // insert the max token

                /**
                 *
                 * Implementing inplace freq update
                 *    __
                 * [ abcbd ]
                 *
                 * let max token be [ bc ]
                 *
                 * [ aX Xb d ]
                 *
                 * remove [b,c] token from the map
                 * now add new token [ aX , Xb ]
                 */

                for( auto &values : m_compiled_object | std::views::values )
                    update_encode(values,max_freq_tok);                  
                   

            } // Epoch

            // clear the compile objects
            m_compiled_object.clear();

        } // train

        /**
         *   encode
         *       Tokenize and Encode the given string
         *   @param
         *       input_string : the string to be encoded
         *   @returns
         *       numpy array [py::array_t<uint32_t>]
         */
        PYBIND11_NOINLINE virtual py::array_t<uint32_t> encode(py::str &input_string) const final
        {
            if (py::len(input_string) < 2)
                py::value_error("The size of the string must be greater than or equal to 2.");

            std::string_view encoded_string = input_string.attr("encode")("UTF8").cast<std::string_view>();
            
            std::vector<uint32_t> encoded_vect(encoded_string.size());
            std::ranges::copy(encoded_string.begin(),encoded_string.end(),encoded_vect.begin());

            size_t size = m_bpe_table.encode(encoded_vect);

            uint32_t *result_vector = new uint32_t[size] ;
            memcpy(result_vector, encoded_vect.data(), size * sizeof result_vector[0]);

            py::capsule encoded_capsule(result_vector, [](void *ptr) { delete static_cast<double*>(ptr);} );

            py::array_t<uint32_t> encoded_text_array({size}, {sizeof result_vector[0]},result_vector, encoded_capsule);
            return encoded_text_array;

        } // encode

        /**
         *   decode
         *       Decode the given encoded string to  original form
         *   @param
         *       encoded_text : encoded string to convert
         */
        PYBIND11_NOINLINE virtual py::bytes decode(py::array_t<uint32_t> encoded_text) const final
        {
            // Request a buffer info (safe access)
            py::buffer_info buf = encoded_text.request();

            // Sanity check
            if (buf.ndim != 1)
                throw std::runtime_error("Expected a 1D NumPy array");

            // Create vector from buffer
            uint32_t *ptr = static_cast<uint32_t *>(buf.ptr);
            std::vector<uint32_t> encoded_vec(ptr, ptr + buf.shape[0]);

            auto result_string = m_bpe_table.decodeToken(encoded_vec);
            return result_string;

        } // decode

        /**
         *   toDict
         *       Dumps the Table to to a py dictionary
         *   @returns
         *       py::dict : dictionary of the objects
         */
        PYBIND11_NOINLINE virtual py::dict toDict() final
        {
            return m_bpe_table.toDict();
        } // toDict
        
        /**
         * pattern 
         *      (Getter <= pattern) 
         */
        PYBIND11_NOINLINE virtual const py::str pattern() const final {
            return m_pattern;
        }
        
        /**
         * vocabCap 
         *      (Getter <= capacity) 
         */
        PYBIND11_NOINLINE virtual py::int_ vocabCap() const final {
            return m_bpe_table.vocabCap();
        }
        
        /**
         * updateVocabCap 
         *      (Setter => vocabCap [capacity]) 
         */
        PYBIND11_NOINLINE virtual void updateVocabCap(const py::int_ &new_size) final {
            m_bpe_table.updateVocabCap(new_size);
        }   
        
        /**
         * vocabSize 
         *      (Getter <= vocabSize) 
         */
        PYBIND11_NOINLINE virtual const py::int_ vocabSize() const final {
            return m_bpe_table.currSize();
        }


    protected:

        PYBIND11_NOINLINE virtual void encodeInplace(
            std::vector<uint32_t> &from,
            uint32_t &size,
            const Token &added_tok) noexcept(true) final
        {
            size_t i; // counter

            uint32_t final_size = 0;

            // Encode Inplace [ could be done using same container but will be bit complex ]
            for (i = 0; i < size - 1; i++)
            {
                auto curr_word = from[i];
                auto next_word = from[i + 1];

                
                if (!(added_tok == std::pair{curr_word, next_word})) // The Token Does not Present
                {
                    from[final_size++] = curr_word;
                    continue;
                }

                auto token_index = added_tok.getId();

                if (final_size > 0) // is there any token before
                {
                    auto &prev_gen_word = from[final_size - 1];
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

                    auto prev_token_pair = Token{prev_gen_word, curr_word};
                    // New Token
                    Token next_token_pair(prev_gen_word, token_index);
                    {
                        std::scoped_lock lock(m_curr_token_mutex);
                        m_curr_token_map[prev_token_pair]--;
                        m_curr_token_map[next_token_pair]++; // Increase the Count of the Freq new type
                    }
                }

                // Add The Max Pair
                {
                    std::scoped_lock lock(m_curr_token_mutex);
                    size_t _c = m_curr_token_map[added_tok]--;
                }
                from[final_size++] = token_index;

                // if there is Next token Then do
                if ((i + 2) < size)
                {
                    Token prev_future_token(next_word, from[i + 2]);
                    Token current_future_token(token_index, from[i + 2]);
                    {
                        std::scoped_lock lock(m_curr_token_mutex);
                        m_curr_token_map[prev_future_token]--;
                        m_curr_token_map[current_future_token]++;
                    }
                }

                // jump the next word
                i += 1;
            }

            if (i < size)
                from[final_size++] = from[i];

            size = final_size;
        }


    private:
        pybind11::module_ m_regex_module;
        pybind11::object m_regex_pattern_compiled;
        std::unordered_map<std::string_view, Sent_Vect_T> m_compiled_object;
        BPE::BPE_Table m_bpe_table;
        BPE::Dictionary<BPE::Token, Freq_T> m_curr_token_map;
        std::mutex m_curr_token_mutex;
        py::str m_pattern; 

        size_t m_vocab_size;

        const std::unordered_set<std::string> m_special_symbol = {

            // This Token will not merge

            // end and begin of the prompt or text input
            "<|endoftext|>",   // EOS
            "<|startoftext|>", // SOS

            // start and end of an message  [ use case for user and assistant text ]
            "<|im_start|>",
            "<|im_end|>",

            // unkown token
            "<|unk|>",

            // separtor between inputs
            "<|sep|>",

            // to specify whether the following text belongs to the assistant or the use
            "<|usr|>",
            "<|assistant|>",

            // padding for meeting the token count
            "<|padding|>",

        };

        // const std::string GPT_STR = GPT4_PATTERN;
    };

    
    PYBIND11_NOINLINE PyBytePairEncoding LoadFromJson(py::dict data) noexcept(false)  {
        throw std::runtime_error("Not Implemented");    
    }

}
# endif // _BPE_H_
