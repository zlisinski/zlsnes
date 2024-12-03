#pragma once

#include "Zlsnes.h"

class Cartridge;

class InfoInterface
{
public:
    virtual void SetMemory(uint8_t *memory) = 0;
    virtual void UpdateCartridgeInfo(const Cartridge &cartridge) = 0;
};