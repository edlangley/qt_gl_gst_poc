#ifndef LOGGER_H
#define LOGGER_H

#include <libgen.h>
#include <QMap>

class Logger
{
public:
    enum LogLevel
    {
        Error,
        Warning,
        Info,
        Debug1,
        Debug2
    };

    Logger();
    void SetModuleLogLevel(unsigned int module, LogLevel level);
    LogLevel GetModuleLogLevel(unsigned int module);
    void LogMessage(unsigned int module, LogLevel severity, const char* const format, ...);
    void LogMessageWithFuncTrace(unsigned int module, LogLevel severity,
                                 const char* const filename, const char* const function, const int line,
                                 const char* const format,
                                 ...);

private:
    void outputMessage(unsigned int module, LogLevel severity, const char* const message);
    QMap<unsigned int, LogLevel> m_currentLogLevels;
};

#define DEFAULT_LOG_LEVEL  Logger::Warning


// The global object actually used for LOG calls
extern Logger GlobalLog;

#define LOG(moduleId, severity, ...)           \
    GlobalLog.LogMessageWithFuncTrace(moduleId, severity,        \
        basename(__FILE__),                                   \
        __PRETTY_FUNCTION__,                               \
        __LINE__,                                   \
        __VA_ARGS__                                 \
    )

// Global log module directory:
enum
{
    LOG_GL,
    LOG_GLSHADERS,
    LOG_OBJLOADER,
    LOG_VIDPIPELINE
};


#endif // LOGGER_H
