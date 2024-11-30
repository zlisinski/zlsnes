#pragma once

#include "Zlsnes.h"
#include "Bytes.h"

class Address
{
public:
    Address() :
        bank(0),
        offset(0)
    {}

    explicit Address(uint32_t addr) :
        bank(Bytes::GetByte<2>(addr)),
        offset(addr & 0xFFFF)
    {}

    Address(uint8_t bank, uint16_t offset) :
        bank(bank),
        offset(offset)
    {}

    explicit operator uint32_t() const
    {
        return Bytes::Make24Bit(bank, offset);
    }

    uint32_t ToUint() const
    {
        return Bytes::Make24Bit(bank, offset);
    }

    uint8_t GetBank() const
    {
        return bank;
    }

    uint16_t GetOffset() const
    {
        return offset;
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