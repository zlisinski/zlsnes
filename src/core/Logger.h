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

// Instruction level logs are very verbose, this allows to turn them on/off for each subsystem.
enum class InstructionLogLevel
{
    eCpu = 0x01,
    ePpu = 0x02,
    eMemory = 0x04,
    eInput = 0x08,
    eTimer = 0x10,
    eInterrupt = 0x20
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
    static void Log(LogLevel level, const char *format, ...)
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

    static void InstructionLog(InstructionLogLevel level, const char *format, ...)
    {
        if ((logLevel == LogLevel::eInstruction) && (instructionLogLevel & static_cast<uint32_t>(level)) && (loggerOutput != NULL))
        {
            va_list args;
            va_start(args, format);

            char buf[1024];
            vsnprintf(buf, sizeof(buf), format, args);

            loggerOutput->Output(std::unique_ptr<LogEntry>(new LogEntry(LogLevel::eInstruction, buf)));

            va_end(args);
        }
    }

    static void SetOutput(LoggerOutput *output) {loggerOutput = output;}
    static void SetLogLevel(LogLevel level) {logLevel = level;}
    static void SetInstructionLogLevel(uint32_t level) {instructionLogLevel = level;}
    static LogLevel GetLogLevel() {return logLevel;}
    static uint32_t GetInstructionLogLevel() {return instructionLogLevel;}

private:
    // For now I only need one output at a time.
    // If needed, change this in the future to support logging to more than one location.
    static LoggerOutput *loggerOutput;

    static LogLevel logLevel;
    static uint32_t instructionLogLevel;
};

#define LogError(...)       do {Logger::Log(LogLevel::eError, __VA_ARGS__);} while (0)
#define LogWarning(...)     do {Logger::Log(LogLevel::eWarning, __VA_ARGS__);} while (0)
#define LogInfo(...)        do {Logger::Log(LogLevel::eInfo, __VA_ARGS__);} while (0)
#define LogDebug(...)       do {Logger::Log(LogLevel::eDebug, __VA_ARGS__);} while (0)
#define LogInstruction(...) do {Logger::Log(LogLevel::eInstruction, __VA_ARGS__);} while (0)

// Subsystem specific instruction-level logs.
#define LogCpu(...)         do {Logger::InstructionLog(InstructionLogLevel::eCpu, __VA_ARGS__);} while (0)
#define LogPpu(...)         do {Logger::InstructionLog(InstructionLogLevel::ePpu, __VA_ARGS__);} while (0)
#define LogMemory(...)      do {Logger::InstructionLog(InstructionLogLevel::eMemory, __VA_ARGS__);} while (0)
#define LogInput(...)       do {Logger::InstructionLog(InstructionLogLevel::eInput, __VA_ARGS__);} while (0)
#define LogTimer(...)       do {Logger::InstructionLog(InstructionLogLevel::eTimer, __VA_ARGS__);} while (0)
#define LogInterrupt(...)   do {Logger::InstructionLog(InstructionLogLevel::eInterrupt, __VA_ARGS__);} while (0)
#define LogAudio(...)       do {} while (0)