
#pragma once

#include <BPE_Tokens.hpp>
# include <fstream>

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
        typedef std::pair<std::vector<uint32_t> *, uint32_t> Word_Vect_T;
        typedef std::vector<std::pair<std::vector<uint32_t> *, uint32_t>> Sent_Vect_T;

        using Freq_T = uint32_t;

    public:
        PYBIND11_NOINLINE PyBytePairEncoding(
            size_t vocabSize,
            pybind11::str pattern = GPT4_PATTERN) __THROW
            : m_bpe_table(vocabSize),
              m_vocab_size(vocabSize)
        {
            try
            {
                m_regex_module = pybind11::module_::import("regex");
            }
            catch (...)
            {
                pybind11::import_error(R"(Can't Import `Regex`. `Regex` module is required to work porperly.)");
            }
            m_regex_pattern_compiled = m_regex_module.attr("compile")(pattern);
        }

        PYBIND11_NOINLINE virtual ~PyBytePairEncoding()
        {
            clearCompileObject();
        }

        /**
         *	compile method
         *		compile the file into a vector format for futher processing
         *	@param
         *		inputString : the string to be compiled
         *		fName : the file name
         */
        PYBIND11_NOINLINE virtual void compile(pybind11::str inputString, pybind11::str fName) noexcept(true) final 
        {
            Sent_Vect_T *sent_vect; // final vector of utf-8 words of the string sentence
            // to delete the sent vect
            auto clear_sent = [](Sent_Vect_T *ptr) -> void
            {
                for (auto &[i,j] : *ptr)
                    // remove the the words
                    delete i;
                // remove the sent_vect
                delete ptr;
                return;
            };
            try
            {
                // tokenizes the sentence to chunks of words
                py::list tokenize_words = m_regex_pattern_compiled.attr("findall")(inputString);
                uint32_t word_count /*number of word chunks*/ =
                    tokenize_words.attr("__len__")().cast<uint32_t>();

                // create new sentence and revese the required amount of space
                sent_vect = new Sent_Vect_T();
                sent_vect->reserve(word_count);

                

                // insert the word vect to the sentence vect
                for (auto words : tokenize_words)
                {
                    std::string_view string_words;
                    std::vector<uint32_t> *BufferVect = new std::vector<uint32_t>();
                    try
                    {
                        string_words = words.cast<std::string_view>(); // This can thow Error Too
                        // convert to utf8
                        if (!BPE::to_utf8_bytes<uint32_t>(string_words, *BufferVect))
                            throw py::value_error();
                        sent_vect->push_back({BufferVect, BufferVect->size()});
                    }
                    catch (py::value_error e) // Error Occured when converting to utf-8
                    {
                        Log(LOG_ERROR, "Can't Convert to utf8.");
                        delete BufferVect;
                        clear_sent(sent_vect);
                    }
                    catch (py::cast_error e) // Error Occured when converting to string_view from string
                    {
                        // very unlikely to happen
                        Log(LOG_ERROR, "Can't Convert to string_view from string.");
                        delete BufferVect;
                        clear_sent(sent_vect);
                    }
                }
                // saving the compiled word
                m_compiled_object[fName.cast<std::string_view>()] = sent_vect;
            }
            catch (...)
            {
                auto file_name = fName.cast<std::string_view>();
                Log(LOG_ERROR, "Unkown Error Occured while compiling %s.", file_name.data());
                // clearing the word
                clear_sent(sent_vect);
            }
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
            std::mutex batch_task_mutex;
            std::condition_variable batch_task_cond_var;
            uint32_t n_batch_finished;

            /*poolig the thread*/
            boost::asio::thread_pool pool(std::thread::hardware_concurrency());

            for (auto &[id, values] : m_compiled_object)
            {
                for (auto &[from, size] : *values)
                {
                    /**
                     * Calculate the Frequence Table  for the first time
                     */
                    auto &From = *from;
                    // Insert the Token and Calculate the Frequency
                    for (size_t i = 0; i < size - 1; i++)
                    {
                        BPE::Token T(From[i], From[i + 1]);
                        m_curr_token_map[T]++;
                    }
                }
            }

            for (size_t epoch = 0; epoch < m_vocab_size - 0x100; epoch++)
            {
                /**
                 * Non-parallelizable work
                 */
                auto &max_freq = *std::max_element(m_curr_token_map.begin(),
                                                   m_curr_token_map.end(),
                                                   [](const std::pair<BPE::Token, uint32_t> &A, const std::pair<BPE::Token, uint32_t> &B) -> bool
                                                   {
                                                       return A.second < B.second;
                                                   });
                // unwrap it max freq
                auto max_freq_tok = max_freq.first;
                auto max_freq_fnum = max_freq.second;

                // Termination Condition
                if (max_freq_fnum < 2)
                {
                    Log(LOG_WARN, "The Min frequency Has been Reached Terminating Training at %zu Epoch", epoch);
                    break;
                }
                // New Added Token
                auto &added_tok = m_bpe_table.addToken(max_freq_tok); // insert the max token

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

                // parallelized task
                std::function update_encode = [
                    this, &added_tok
                ](
                    Sent_Vect_T *vec , 
                    std::condition_variable &cond_var, 
                    std::mutex &mutex ,
                    uint32_t &counter
                ) -> void
                {
                    for (auto &[From, size] : *vec)
                    {
                        std::vector<uint32_t> &from = *From;
                        encodeInplace(from, size, added_tok);

                    } // For Total String
                    {
                        std::scoped_lock lock(mutex);
                        counter++;
                    }
                    cond_var.notify_one();
                };

                n_batch_finished = 0;
                for (auto &[id, values] : m_compiled_object)
                {
                    /**
                     * parallelizable part
                     */
                    boost::asio::post(pool,
                        std::bind(
                                      [update_encode, &values](
                                        std::condition_variable &batch_task_cond_var, 
                                        std::mutex &batch_task_mutex ,
                                        uint32_t &n_batch_finished
                                      ) -> void
                                      {
                                          update_encode(values,batch_task_cond_var,batch_task_mutex,n_batch_finished);
                                      },
                                      std::ref(batch_task_cond_var),
                                      std::ref(batch_task_mutex),
                                      std::ref(n_batch_finished)
                                )
                    );

                } // For Each Compiled Sentence


                {
                    std::unique_lock lock(batch_task_mutex);
                    batch_task_cond_var.wait(lock, [this,&n_batch_finished]() -> bool
                                            {
                                                return n_batch_finished == m_compiled_object.size(); 
                                            });
                }

                /* Dump the table and clear the table */
                if (epoch % 100 == 0)
                {
                    for (auto it = m_curr_token_map.begin(); it != m_curr_token_map.end();)
                        it = it->second /*freq*/ == 0 ? m_curr_token_map.erase(it) : ++it;

                    /*save the bpe_table*/
                }

            } // Epoch

            // join the threads [ no need for the pool ]
            pool.join();

            // clear the compile objects
            clearCompileObject();

        } // train

        /**
        *   encode
        *       Tokenize and Encode the given string 
        *   @param
        *       input_string : the string to be encoded 
        *   @returns 
        *       numpy array [py::array_t<uint32_t>]
        */
        PYBIND11_NOINLINE virtual py::array_t<uint32_t> encode(py::str input_string) const final  
        {
            std::string_view string_words;
            std::vector<uint32_t> *BufferVect = new std::vector<uint32_t>();
            try
            {
                string_words = input_string.cast<std::string_view>(); // This can thow Error Too
                // convert to utf8
                if (!BPE::to_utf8_bytes<uint32_t>(string_words, *BufferVect))
                    throw -1;
            }
            catch (int e) // Error Occured when converting to utf-8
            {
                Log(LOG_ERROR, "Can't Convert to utf8.");
                delete BufferVect;
                throw py::value_error("Bad String.");
            }
            catch (py::cast_error e) // Error Occured when converting to string_view from string
            {
                // very unlikely to happen
                Log(LOG_ERROR, "Can't Convert to string_view from string.");
                delete BufferVect;
                throw e;
            }

            size_t size = m_bpe_table.encode(*BufferVect);

            uint32_t *result_vector = new uint32_t[size];
            memcpy(result_vector,BufferVect->data(),size * sizeof result_vector[0]);

            py::capsule encoded_capsule(result_vector,[](void *ptr) -> void {
                uint32_t* buff_ptr = (uint32_t*)ptr;
                delete buff_ptr;
            });

            delete BufferVect;

            py::array_t<uint32_t> encoded_text_array(size,result_vector,encoded_capsule);
            return encoded_text_array;

        } // encode 

        /**
        *   decode
        *       Decode the given encoded string to  original form 
        *   @param 
        *       encoded_text : encoded string to convert 
        */
        PYBIND11_NOINLINE virtual py::str decode(py::array_t<uint32_t> encoded_text)  const final  
        {
             // Request a buffer info (safe access)
            py::buffer_info buf = encoded_text.request();

            // Sanity check
            if (buf.ndim != 1)
                throw std::runtime_error("Expected a 1D NumPy array");

            // Create vector from buffer
            uint32_t* ptr = static_cast<uint32_t*>(buf.ptr);
            std::vector<uint32_t> encoded_vec(ptr, ptr + buf.shape[0]);

            auto result_string = m_bpe_table.decode_token(encoded_vec);
            return result_string;

        } // decode 

        /**
        *   save
        *       Dumps the Table to the file for future reference
        *   @param 
        *       dest : destiation file 
        *   @returns 
        *       bool : if the file is saved or not 
        */
        PYBIND11_NOINLINE virtual bool save(py::str dest) const final  
        {

        } // save 

        /**
        *   displayTable 
        *       Prints the table to the standard output or given filename 
        */
        PYBIND11_NOINLINE virtual void displayTable(std::string_view file_name = "") const final  
        {
            if(file_name == "") {
                m_bpe_table.printToken();
            } else {
                std::ofstream fp_out(file_name.data());
                if(!fp_out)
                {
                    Log(LOG_ERROR,"Object Does't Saved");
                    Log(LOG_ERROR,"Can't Open File %s ",file_name.data());
                    return ;
                }
                m_bpe_table.printToken(fp_out);
            }
        } // displayTable 

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
                
                if (!(added_tok == std::pair{curr_word,next_word})) // The Token Does not Present
                {
                    from[final_size++] = curr_word;
                    continue;
                }

                auto token_index = added_tok.getId();

                {
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

                        auto prev_token_pair = Token(prev_gen_word, curr_word);
                        // New Token
                        Token next_token_pair(prev_gen_word, token_index);
                        {
                            boost::mutex::scoped_lock lock(m_curr_token_mutex);
                            m_curr_token_map[prev_token_pair]--;
                            m_curr_token_map[next_token_pair]++; // Increase the Count of the Freq new type
                        }
                    }

                    // Add The Max Pair
                    {
                        boost::mutex::scoped_lock lock(m_curr_token_mutex);
                        m_curr_token_map[added_tok]--;
                    }
                    from[final_size++] = token_index;

                    // if there is Next token Then do
                    if ((i + 2) < size)
                    {
                        Token prev_future_token(next_word, from[i + 2]);
                        Token current_future_token(token_index, from[i + 2]);
                        {
                            boost::mutex::scoped_lock lock(m_curr_token_mutex);
                            m_curr_token_map[prev_future_token]--;
                            m_curr_token_map[current_future_token]++;
                        }
                    }
                }
                
                // jump the next word
                i += 1;
            }

            if (i < size)
                from[final_size++] = from[i];

            size = final_size;
        }

        /**
        *   clearCompileObject
        *       Clear all the Compile object from the list
        *   ERROR::  
        *       Might Throw if there is any dangling pointer 
        */
        PYBIND11_NOINLINE virtual void clearCompileObject() __THROW final
        {
            for (auto &[k, v] : m_compiled_object)
            {
                for (auto &[p1, p2] : *v)
                    delete p1;
                delete v;
            }
            m_compiled_object.clear();
        }

    private:
        pybind11::module_ m_regex_module;
        pybind11::object m_regex_pattern_compiled;
        boost::unordered_map<std::string_view, Sent_Vect_T *> m_compiled_object;
        BPE::BPE_Table m_bpe_table;
        BPE::Dictionary<BPE::Token, Freq_T> m_curr_token_map;
        boost::mutex m_curr_token_mutex;

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

}
