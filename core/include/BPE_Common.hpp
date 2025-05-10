
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
# include <ranges>

# include <unordered_map>
# include <unordered_set>

// c-based libs 
# include <cstring>
# include <cassert>
# include <cstdlib>


// Third party libs 
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
        
} // namespace BPE
    
#define TODO(Func)                                                                                   \
    Log(BPE::LOG_WARN, "File [ %s ] : Line [ %zu ] : Not Implementd :: " #Func, __FILE__, __LINE__); \
    abort()

# include "BPE_EncodingPatterns.hpp"