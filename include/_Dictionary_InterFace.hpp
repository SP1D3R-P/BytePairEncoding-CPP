
#pragma once

#include <type_traits>
#include <string>
#include <unordered_map>

namespace BPE
{
    template <typename KV, typename V, typename HASH>
    class Dictionary;

    template <typename Derived>
    class DictionaryKeyInterface
    {
    public:
        DictionaryKeyInterface() {}
        virtual ~DictionaryKeyInterface() {}

        /**
         * All Assign Operators
         */

        virtual Derived &operator=(const Derived &B) = 0;

        /**
         * All Compare Operators
         */
        virtual bool operator==(const Derived &B) const
        {
            (void)B;
            return true;
        }

        struct HashFunction
        {
            virtual uint64_t operator()(const Derived &B) const final
            {
                return ((DictionaryKeyInterface *)(&B))->__hash__();
            }
        };

    private:
        /**
         * Hash Function 
         */
        virtual uint64_t __hash__() = 0;

        template <typename KV, typename V, typename HASH>
        friend class BPE::Dictionary;
    };

    template <typename KV, typename V, typename HASH = typename DictionaryKeyInterface<KV>::HashFunction>
    class Dictionary : public std::unordered_map<KV, V, HASH>
    {

    public:
        Dictionary()
        {
            static_assert(std::is_base_of<DictionaryKeyInterface<KV>, KV>::value == true,
                          "ERROR :: Key Type(Class) Must be Inherited from DictKeyInterface");

            // hash_function
        }

        ~Dictionary() {}
    };
}