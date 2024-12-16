#include "Logger.h"

LoggerOutput *Logger::loggerOutput = NULL;
LogLevel Logger::logLevel = LogLevel::eError;
uint32_t Logger::instructionLogLevel = 0;