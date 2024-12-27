#pragma once

#include "Zlsnes.h"

class Cartridge;
class Ppu;

class InfoInterface
{
public:
    virtual void SetIoPorts21(const uint8_t *ioPorts21) = 0;
    virtual void SetPpu(const Ppu *ppu) = 0;

    virtual void UpdateCartridgeInfo(const Cartridge &cartridge) = 0;
};