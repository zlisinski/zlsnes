#pragma once

#include "Zlsnes.h"

class Buttons
{
public:
    enum Button
    {
        eButtonNone = 0x00,
        eButtonUp = 0x01,
        eButtonRight = 0x02,
        eButtonDown = 0x04,
        eButtonLeft = 0x08,
        eButtonSelect = 0x10,
        eButtonStart = 0x20,
        eButtonB = 0x40,
        eButtonA = 0x80,
        eButtonY = 0x100,
        eButtonX = 0x200,
        eButtonL = 0x400,
        eButtonR = 0x800
    };

    Buttons() : data(0) {}

    bool IsUpPressed() {return data & eButtonUp;}
    bool IsRightPressed() {return data & eButtonRight;}
    bool IsDownPressed() {return data & eButtonDown;}
    bool IsLeftPressed() {return data & eButtonLeft;}
    bool IsSelectPressed() {return data & eButtonSelect;}
    bool IsStartPressed() {return data & eButtonStart;}
    bool IsBPressed() {return data & eButtonB;}
    bool IsAPressed() {return data & eButtonA;}
    bool IsYPressed() {return data & eButtonY;}
    bool IsXPressed() {return data & eButtonX;}
    bool IsLPressed() {return data & eButtonL;}
    bool IsRPressed() {return data & eButtonR;}

    uint16_t data;
};