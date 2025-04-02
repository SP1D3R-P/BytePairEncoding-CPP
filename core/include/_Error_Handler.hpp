

#pragma once
#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#else
#if LOG_LEVEL > 3
#error "Log Level Can't Be Greater than 2"
#endif // LOG_LEVEL
#endif // LOG_LEVEL

# include <common.hpp>

#define Log(lv, msg, ...)                                                                        \
    do                                                                                           \
    {                                                                                            \
        if (lv >= LOG_LEVEL)                                                                     \
            fprintf(stdout, "%s :: " msg ".\n", BPE::_LogString[lv] __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

namespace BPE
{
    enum Log_Level
    {
        LOG_DEBUG = 0x0,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR
    };

    class Error
    {

    public:
        Error(std::string &&Type, const std::string &Desc, const char *file)
            : m_what(Desc),
              m_file(file),
              m_type(Type)
        {
        }

        virtual ~Error()
        {
        }

        operator const char *() const
        {
            return m_what.c_str();
        }

        void log() const
        {
            fprintf(stdout, "%s:: %s : %s.\n",
                    m_type.c_str(),
                    m_file.c_str(),
                    m_what.c_str());
        }

    private:
        const std::string m_what;
        const std::string m_file;
        const std::string m_type;
    };

    class TimeIt
    {
    public:
        using Clock = std::chrono::high_resolution_clock;

        // Constructor starts the timer with display control
        TimeIt(const std::string &taskName = "Execution")
            : m_taskName(taskName) {}

        ~TimeIt()
        {
        }
        
        void Start()
        {
            m_startTime = Clock::now();
        }

        void End()
        {
            auto endTime = Clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count();

            std::cout << m_taskName << " took " << ((float)duration) / 1000 << " sec.\n\n";
        }


    private:
        std::string m_taskName;
        std::chrono::time_point<Clock> m_startTime;
    };

    static char *_LogString[4] = {
        "DEBUG",
        "INFO ",
        "WARN ",
        "ERROR"};

#define ERROR(T, what, ...) throw Error(#T, what, __FILE__)
#define TODO(Func)                                                                                   \
    Log(BPE::LOG_WARN, "File [ %s ] : Line [ %zu ] : Not Implementd :: " #Func, __FILE__, __LINE__); \
    abort()

} // namespace BPE
