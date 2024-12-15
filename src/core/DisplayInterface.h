#pragma once

#include <string>

#include "Zlsnes.h"

const int SCREEN_X = 512;
const int SCREEN_Y = 480;

class DisplayInterface
{
public:
    DisplayInterface() {}

    virtual void FrameReady(const std::array<uint32_t, SCREEN_X * SCREEN_Y> &frameBuffer) = 0;
    virtual void RequestMessageBox(const std::string &message) = 0;

protected:
    ~DisplayInterface() {}
};