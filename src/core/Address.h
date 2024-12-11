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

    Address(uint8_t bank, uint8_t offsetHigh, uint8_t offsetLow) :
        bank(bank),
        offset(Bytes::Make16Bit(offsetHigh, offsetLow))
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

    bool operator==(const Address &a) const {return (bank == a.bank) && (offset == a.offset);}
    bool operator!=(const Address &a) const {return (bank != a.bank) || (offset != a.offset);}
    bool operator<(const Address &a) const {return ToUint() < a.ToUint();}
    bool operator<=(const Address &a) const {return ToUint() <= a.ToUint();}
    bool operator>(const Address &a) const {return ToUint() > a.ToUint();}
    bool operator>=(const Address &a) const {return ToUint() >= a.ToUint();}
    bool operator==(uint32_t a) const {return ToUint() == a;}
    bool operator!=(uint32_t a) const {return ToUint() != a;}
    bool operator<(uint32_t a) const {return ToUint() < a;}
    bool operator<=(uint32_t a) const {return ToUint() <= a;}
    bool operator>(uint32_t a) const {return ToUint() > a;}
    bool operator>=(uint32_t a) const {return ToUint() >= a;}

private:
    uint8_t bank;
    uint16_t offset;
};