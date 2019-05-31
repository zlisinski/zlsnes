#pragma once

#include <string>

#include "Zlsnes.h"

class DisplayInterface
{
public:
    DisplayInterface() {}

    virtual void FrameReady(uint32_t *frameBuffer) = 0;
    virtual void RequestMessageBox(const std::string &message) = 0;

protected:
    ~DisplayInterface() {}
};