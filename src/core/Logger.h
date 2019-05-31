#pragma once

#include <memory>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <sys/time.h>

enum class LogLevel
{
    eError,
    eWarning,
    eInfo,
    eDebug,
    eInstruction
};


class LogEntry
{
public:
    LogEntry(LogLevel level, const std::string &message) :
        level(level),
        message(message)
    {
        gettimeofday(&tv, NULL);
    }

    timeval tv;
    LogLevel level;
    std::string message;
};


class LoggerOutput
{
public:
    virtual void Output(std::unique_ptr<LogEntry> entry) = 0;
};


class Logger
{
public:
    static inline void Log(LogLevel level, const char *format, ...)
    {
        if (level <= logLevel && loggerOutput != NULL)
        {
            va_list args;
            va_start(args, format);

            char buf[1024];
            vsnprintf(buf, sizeof(buf), format, args);

            loggerOutput->Output(std::unique_ptr<LogEntry>(new LogEntry(level, buf)));

            va_end(args);
        }
    }

    static void SetOutput(LoggerOutput *output) {loggerOutput = output;}
    static void SetLogLevel(LogLevel level) {logLevel = level;}
    static LogLevel GetLogLevel() {return logLevel;}

private:
    // For now I only need one output at a time.
    // If needed, change this in the future to support logging to more than one location.
    static LoggerOutput *loggerOutput;

    static LogLevel logLevel;
};

#define LogError(...) do {Logger::Log(LogLevel::eError, __VA_ARGS__);} while (0)
#define LogWarning(...) do {Logger::Log(LogLevel::eWarning, __VA_ARGS__);} while (0)
#define LogInfo(...) do {Logger::Log(LogLevel::eInfo, __VA_ARGS__);} while (0)
#define LogDebug(...) do {Logger::Log(LogLevel::eDebug, __VA_ARGS__);} while (0)
#define LogInstruction(...) do {Logger::Log(LogLevel::eInstruction, __VA_ARGS__);} while (0)
#define LogAudio(...) do {} while (0)