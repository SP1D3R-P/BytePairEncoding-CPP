
# pragma once 

// standard cpp libs 
# include <iostream>
# include <vector>
# include <string>
# include <algorithm>
# include <chrono>
# include <type_traits>
# include <fstream>
# include <sstream>


// Might not use in future and switch to boost 
# include <unordered_map>
# include <unordered_set>

// c-based libs 
# include <cstring>
# include <cassert>
# include <cstdlib>

// icu [ for utf conversion ]
#include <unicode/unistr.h>
#include <unicode/ucnv.h>

// Third party libs 
# include <boost/regex.hpp>
# include <boost/unordered_map.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>


# include <pybind11/pybind11.h>
# include <pybind11/numpy.h>
# include <pybind11/embed.h>


#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#else
#if LOG_LEVEL > 3
#error "Log Level Can't Be Greater than 3"
#endif // LOG_LEVEL
#endif // LOG_LEVEL


# include "BPE_DictionaryInterFace.hpp"
# include "BPE_MemoryManagement.hpp"

#define Log(lv, msg, ...)                                                                              \
    do                                                                                                 \
    {                                                                                                  \
        if (lv >= LOG_LEVEL)                                                                           \
            fprintf(stdout, "%s :: " msg ".\n", BPE::_logging_string_[lv] __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

namespace BPE
{
    enum BPE_Log_Level
    {
        LOG_DEBUG = 0x0,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        __BPE_LOG_LEVEL_COUNT__,
    };

    static const char *_logging_string_[__BPE_LOG_LEVEL_COUNT__] = {
        "DEBUG",
        "INFO ",
        "WARN ",
        "ERROR",
    };

    template <typename T = uint32_t>
    static bool to_utf8_bytes(const std::string_view &input, std::vector<T> &retVect)
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

            retVect.clear();

            if (retVect.capacity() < utf8_str.length())
                retVect.reserve(utf8_str.length());

            for (size_t i = 0; i < utf8_str.length(); i++)
                retVect.push_back((T)(unsigned char)utf8_str[i]);
        } catch (const std::exception &e) {
            std::cerr << "Error during UTF-8 conversion: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

        
} // namespace BPE
    
#define TODO(Func)                                                                                   \
    Log(BPE::LOG_WARN, "File [ %s ] : Line [ %zu ] : Not Implementd :: " #Func, __FILE__, __LINE__); \
    abort()

# include "BPE_EncodingPatterns.hpp"