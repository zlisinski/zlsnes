#pragma once

#include "Zlsnes.h"
#include "IoRegisterProxy.h"

class Memory;

class Ppu : public IoRegisterProxy
{
public:
    Ppu(Memory *memory);
    virtual ~Ppu() {}

    // Inherited from IoRegisterProxy.
    bool WriteByte(uint32_t address, uint8_t byte) override;
    uint8_t ReadByte(uint32_t address) const override;

private:
    Memory *memory;
    std::array<uint8_t, 544> oam;
    std::array<uint8_t, 0xFFFF> vram;
    std::array<uint8_t, 512> palette;

    uint8_t *regINIDISP; // 0x2100 Display Control 1
};