#ifndef ZLSNES_CORE_TESTS_TEST_LOGGER_H
#define ZLSNES_CORE_TESTS_TEST_LOGGER_H

#include "Logger.h"

class TestLogger : public LoggerOutput
{
public:
    TestLogger()
    {
        Logger::SetOutput(this);
        Logger::SetLogLevel(LogLevel::eInstruction);
        Logger::SetInstructionLogLevel(0xFFFFFFFF);
    }
    virtual ~TestLogger() {}
    void Output(std::unique_ptr<LogEntry> entry) override
    {
        printf("%s\n", entry->message.c_str());
    }
};

#endif