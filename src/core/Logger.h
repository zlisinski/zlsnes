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
    eInterrupt = 0x20,
    eApu = 0x40,
    eDma = 0x80,
    eSpc700 = 0x100,
    eSpcMem = 0x200,
    eSpcTimer = 0x400,
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

    static void InstructionLog(const char *label, const char *format, ...)
    {
        if (loggerOutput != NULL)
        {
            va_list args;
            va_start(args, format);

            char buf[1024];
            strncpy(buf, label, sizeof(buf) - 1);
            size_t labelLen = strlen(buf);
            vsnprintf(buf + labelLen, sizeof(buf) - labelLen, format, args);

            loggerOutput->Output(std::unique_ptr<LogEntry>(new LogEntry(LogLevel::eInstruction, buf)));

            va_end(args);
        }
    }

    static void SetOutput(LoggerOutput *output) {loggerOutput = output;}
    static void SetLogLevel(LogLevel level) {logLevel = level;}
    static void SetInstructionLogLevel(uint32_t level) {instructionLogLevel = level;}
    static LogLevel GetLogLevel() {return logLevel;}
    static uint32_t GetInstructionLogLevel() {return instructionLogLevel;}
    static inline bool IsInstLevel(InstructionLogLevel level)
    {
        return (logLevel == LogLevel::eInstruction) &&
               (instructionLogLevel & static_cast<uint32_t>(level)) &&
               (loggerOutput != NULL);
    }

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
#define LogCpu(...)       do {if (Logger::IsInstLevel(InstructionLogLevel::eCpu))       Logger::InstructionLog("Cpu:      ", __VA_ARGS__);} while (0)
#define LogPpu(...)       do {if (Logger::IsInstLevel(InstructionLogLevel::ePpu))       Logger::InstructionLog("Ppu       ", __VA_ARGS__);} while (0)
#define LogMemory(...)    do {if (Logger::IsInstLevel(InstructionLogLevel::eMemory))    Logger::InstructionLog("Memory:   ", __VA_ARGS__);} while (0)
#define LogInput(...)     do {if (Logger::IsInstLevel(InstructionLogLevel::eInput))     Logger::InstructionLog("Input:    ", __VA_ARGS__);} while (0)
#define LogTimer(...)     do {if (Logger::IsInstLevel(InstructionLogLevel::eTimer))     Logger::InstructionLog("Timer:    ", __VA_ARGS__);} while (0)
#define LogInterrupt(...) do {if (Logger::IsInstLevel(InstructionLogLevel::eInterrupt)) Logger::InstructionLog("Int:      ", __VA_ARGS__);} while (0)
#define LogApu(...)       do {if (Logger::IsInstLevel(InstructionLogLevel::eApu))       Logger::InstructionLog("Apu:      ", __VA_ARGS__);} while (0)
#define LogDma(...)       do {if (Logger::IsInstLevel(InstructionLogLevel::eDma))       Logger::InstructionLog("Dma:      ", __VA_ARGS__);} while (0)
#define LogSpc700(...)    do {if (Logger::IsInstLevel(InstructionLogLevel::eSpc700))    Logger::InstructionLog("Spc:      ", __VA_ARGS__);} while (0)
#define LogSpcMem(...)    do {if (Logger::IsInstLevel(InstructionLogLevel::eSpcMem))    Logger::InstructionLog("SpcMem:   ", __VA_ARGS__);} while (0)
#define LogSpcTimer(...)  do {if (Logger::IsInstLevel(InstructionLogLevel::eSpcTimer))  Logger::InstructionLog("SpcTimer: ", __VA_ARGS__);} while (0)