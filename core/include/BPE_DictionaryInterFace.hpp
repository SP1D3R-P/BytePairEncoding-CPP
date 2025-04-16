
#pragma once

namespace BPE
{
    template <typename KV, typename V, typename HASH>
    class Dictionary;
    /**
     * This class Defines How a Dictory Key class should behave 
     * All the Key Class of Dictiary Should Inherit this class
     * @param
     *      Derived : is the Derived Class  
     */
    template <typename Derived>
    class DictionaryKeyInterface
    {
    public:
        DictionaryKeyInterface() {}
        virtual ~DictionaryKeyInterface() {}

        /**
         * All Required methods 
         */

        virtual Derived &operator=(const Derived &B) = 0;
        virtual bool operator==(const Derived &B) const = 0;

        /**
         * This a hidden class which will be used for hasing 
         */
        struct HashFunction
        {
            virtual uint64_t operator()(const Derived &B) const final{ return ((DictionaryKeyInterface *)(&B))->__hash__(); }
        };

    private:
        /**
         * Hash Function ( that will be used for the hashing the key)
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
            /**
             * Making sure the Key Class is of Proper Inheritance
             */
            static_assert(std::is_base_of<DictionaryKeyInterface<KV>, KV>::value == true,
                          "ERROR :: Key Type(Class) Must be Inherited from DictKeyInterface");
        }
        virtual ~Dictionary() {}
    };
}