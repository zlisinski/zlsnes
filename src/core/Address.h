#pragma once

#include "Zlsnes.h"

class Address
{
public:
    Address() :
        bank(0),
        offset(0)
    {}

    explicit Address(uint32_t addr) :
        bank((addr >> 16) & 0xFF),
        offset(addr & 0xFFFF)
    {}

    Address(uint8_t bank, uint16_t offset) :
        bank(bank),
        offset(offset)
    {}

    explicit operator uint32_t() const
    {
        return Make24Bit(bank, offset);
    }

    uint32_t ToUint() const
    {
        return Make24Bit(bank, offset);
    }

    Address AddOffset(uint16_t off) const
    {
        return Address(this->ToUint() + off);
    }

    Address AddOffsetWrapBank(uint16_t off) const
    {
        return Address(bank, offset + off);
    }

private:
    uint8_t bank;
    uint16_t offset;
};