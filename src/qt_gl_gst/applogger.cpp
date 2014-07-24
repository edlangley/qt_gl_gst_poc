#include <stdarg.h>
#include <stdio.h>
#include <sstream>
#include "applogger.h"

Logger GlobalLog;

#define APPLOGGER_MAX_MSG_LEN             256

Logger::Logger()
{
}

void Logger::SetModuleLogLevel(unsigned int module, LogLevel level)
{
    m_currentLogLevels[module] = level;
}

Logger::LogLevel Logger::GetModuleLogLevel(unsigned int module)
{
    if(m_currentLogLevels.contains(module) == false)
    {
        m_currentLogLevels[module] = DEFAULT_LOG_LEVEL;
    }

    return m_currentLogLevels[module];
}

void Logger::LogMessage(unsigned int module, LogLevel severity, const char* const format, ...)
{

    if(m_currentLogLevels.contains(module) == false)
    {
        m_currentLogLevels[module] = DEFAULT_LOG_LEVEL;
    }

    if(severity <= m_currentLogLevels[module])
    {
        va_list args;
        va_start(args, format);

        char buffer[APPLOGGER_MAX_MSG_LEN];
        vsnprintf(buffer, APPLOGGER_MAX_MSG_LEN, format, args);

        // Allow single place to decide how/where to print message
        outputMessage(module, severity, buffer);

        va_end(args);
    }
}

void Logger::LogMessageWithFuncTrace(unsigned int module, LogLevel severity,
                             const char* const filename, const char* const function, const int line,
                             const char* const format,
                             ...)
{
    if(m_currentLogLevels.contains(module) == false)
    {
        m_currentLogLevels[module] = DEFAULT_LOG_LEVEL;
    }

    if(severity <= m_currentLogLevels[module])
    {
        va_list args;
        va_start(args, format);

        std::string logString = filename;
        logString += ":";
        logString += function;
        logString += ":";
        std::ostringstream ss;
        ss << line;
        logString += ss.str();
        logString += "> ";
        logString += format;

        char buffer[APPLOGGER_MAX_MSG_LEN];
        vsnprintf(buffer, APPLOGGER_MAX_MSG_LEN, logString.c_str(), args);

        // Allow single place to decide how/where to print message
        outputMessage(module, severity, buffer);

        va_end(args);
    }
}

void Logger::outputMessage(unsigned int module, LogLevel severity, const char * const message)
{
    if(severity <= Logger::Error)
    {
        qCritical(message);
    }
    else
    {
        qDebug(message);
    }
}
