#pragma once

#include "Zlsnes.h"

class Cartridge;

class InfoInterface
{
public:
    virtual void SetIoPorts21(const uint8_t *ioPorts21) = 0;
    virtual void SetVram(const uint8_t *vram) = 0;
    virtual void SetOam(const uint8_t *oam) = 0;
    virtual void SetCgram(const uint8_t *cgram) = 0;

    virtual void UpdateCartridgeInfo(const Cartridge &cartridge) = 0;
};