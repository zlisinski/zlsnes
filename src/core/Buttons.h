#pragma once

#include "Zlsnes.h"

class Buttons
{
public:
    enum Button
    {
        eButtonNone   = 0x00,

        eButtonR      = 0x10,
        eButtonL      = 0x20,
        eButtonX      = 0x40,
        eButtonA      = 0x80,

        eButtonRight  = 0x100,
        eButtonLeft   = 0x200,
        eButtonDown   = 0x400,
        eButtonUp     = 0x800,

        eButtonStart  = 0x1000,
        eButtonSelect = 0x2000,
        eButtonY      = 0x4000,
        eButtonB      = 0x8000
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