#pragma once

#include <stdexcept>

class NotYetImplementedException : public std::runtime_error
{
public:
    NotYetImplementedException(const std::string &message) : std::runtime_error(message) {}
};


class InfiniteLoopException : public std::runtime_error
{
public:
    InfiniteLoopException() : std::runtime_error("Infinite loop detected.\nEmulation stopped.") {}
};