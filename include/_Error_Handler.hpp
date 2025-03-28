

#pragma once
#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#else
#if LOG_LEVEL > 2
#error "Log Level Can't Be Greater than 2"
#endif //LOG_LEVEL
#endif // LOG_LEVEL

#define Log(lv, msg, ...) do { \
    if (lv >= LOG_LEVEL) \
        fprintf(stdout, "%s :: " msg ".\n" , BPE::Error::m_l_cstr[lv] __VA_OPT__(,) __VA_ARGS__); \
} while (0)

namespace BPE
{
    enum Log_Level
    {
        LOG_INFO = 0x0,
        LOG_WARN,
        LOG_ERROR
    };

    class Error
    {

    public:
        Error(Log_Level _l, const char *note, const char *file)
            : m_what(note),
              m_file(file),
              m_level(_l)
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
                    m_l_cstr[m_level],
                    m_file.c_str(),
                    m_what.c_str());
        }

    private:
        const std::string m_what;
        const std::string m_file;
        Log_Level m_level;

    public:
        static const char *m_l_cstr[3];
    };

    const char *Error::m_l_cstr[3] = {
        "INFO ",
        "WARN ",
        "ERROR"};



#define ERROR(level, what, ...) throw Error(LOG_##level, what, __FILE__)
#define TODO(Func) ERROR(WARN, "Not Implementd :: " #Func, NULL)

} // namespace BPE
