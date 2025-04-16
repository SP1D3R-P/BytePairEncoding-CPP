
#pragma once

namespace BPE
{

    template <typename T>
    class MemoryAllocator 
    {
    public:
        using value_type = T; 
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;


        MemoryAllocator() = default;
        ~MemoryAllocator() = default;

        // Allocate memory for `n` objects
        T *allocate(std::size_t n)
        {
            std::cout << "Allocating " << n << " objects\n";
            if (n == 0)
                return nullptr;
            if (auto ptr = static_cast<T *>(std::malloc(n * sizeof(T))))
            {
                return ptr;
            }
            throw std::bad_alloc(); // Throw if memory allocation fails
        }

        // Deallocate memory
        void deallocate(T *ptr, std::size_t n)
        {
            std::cout << "Deallocating " << n << " objects\n";
            std::free(ptr);
        }

        // Rebind for compatibility with STL containers
        template <typename U>
        struct rebind
        {
            using other = MemoryAllocator<U>;
        };
    };

} // BPE
