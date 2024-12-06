#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Address.h"
#include "Bytes.h"
#include "IoRegisterProxy.h"
#include "Zlsnes.h"


class InfoInterface;
class Timer;


class Memory : public IoRegisterSubject
{
public:
    Memory(InfoInterface *infoInterface = NULL);
    virtual ~Memory();

    void SetRomMemory(std::vector<uint8_t> &gameRomMemory);
    void SetTimer(Timer *timer);

    uint8_t Read8Bit(uint32_t addr) const;
    uint8_t Read8Bit(const Address &addr) const
    {
        return Read8Bit(addr.ToUint());
    }
    // Bypasses special read code. Only use for Debugger.
    uint8_t ReadRaw8Bit(uint32_t addr) const
    {
        return memory[addr];
    }

    // Reads that don't wrap at bank boundaries. E.G. Read16Bit(0x12FFFF) will read from 0x12FFFF and 0x130000.
    uint16_t Read16Bit(uint32_t addr) const
    {
        return Bytes::Make16Bit(Read8Bit(addr + 1), Read8Bit(addr));
    }
    uint16_t Read16Bit(const Address &addr) const
    {
        uint8_t low = Read8Bit(addr);
        uint8_t high = Read8Bit(addr.AddOffset(1));
        return Bytes::Make16Bit(high, low);
    }
    uint32_t Read24Bit(uint32_t addr) const
    {
        uint8_t low = Read8Bit(addr);
        uint8_t mid = Read8Bit(addr + 1);
        uint8_t high = Read8Bit(addr + 2);
        return Bytes::Make24Bit(high, mid, low);
    }
    uint32_t Read24Bit(const Address &addr) const
    {
        uint8_t low = Read8Bit(addr);
        uint8_t mid = Read8Bit(addr.AddOffset(1));
        uint8_t high = Read8Bit(addr.AddOffset(2));
        return Bytes::Make24Bit(high, mid, low);
    }

    // Reads that wrap at bank boundaries. E.G. Read16BitWrapBank(0x12, 0xFFFF) will read from 0x12FFFF and 0x120000.
    uint16_t Read16BitWrapBank(uint8_t bank, uint16_t addr) const
    {
        uint8_t low = Read8Bit(Bytes::Make24Bit(bank, addr));
        uint8_t high = Read8Bit(Bytes::Make24Bit(bank, addr + 1));
        return Bytes::Make16Bit(high, low);
    }
    uint16_t Read16BitWrapBank(const Address &addr) const
    {
        uint8_t low = Read8Bit(addr);
        uint8_t high = Read8Bit(addr.AddOffsetWrapBank(1));
        return Bytes::Make16Bit(high, low);
    }
    uint32_t Read24BitWrapBank(uint8_t bank, uint16_t addr) const
    {
        uint8_t low = Read8Bit(Bytes::Make24Bit(bank, addr));
        uint8_t mid = Read8Bit(Bytes::Make24Bit(bank, addr + 1));
        uint8_t high = Read8Bit(Bytes::Make24Bit(bank, addr + 2));
        return Bytes::Make24Bit(high, mid, low);
    }
    uint32_t Read24BitWrapBank(const Address &addr) const
    {
        uint8_t low = Read8Bit(addr);
        uint8_t mid = Read8Bit(addr.AddOffsetWrapBank(1));
        uint8_t high = Read8Bit(addr.AddOffsetWrapBank(2));
        return Bytes::Make24Bit(high, mid, low);
    }

    void Write8Bit(uint32_t addr, uint8_t value);
    void Write8Bit(const Address &addr, uint8_t value)
    {
        Write8Bit(addr.ToUint(), value);
    }
    void Write16Bit(const Address &addr, uint16_t value)
    {
        Write8Bit(addr.ToUint(), value & 0xFF);
        Write8Bit(addr.AddOffset(1).ToUint(), value >> 8);
    }
    void Write16BitWrapBank(const Address &addr, uint16_t value)
    {
        Write8Bit(addr.ToUint(), value & 0xFF);
        Write8Bit(addr.AddOffsetWrapBank(1).ToUint(), value >> 8);
    }

    // Bypasses checking of reads/writes from/to special addresses. Don't use unless you know what you are doing.
    const uint8_t *GetBytePtr(uint32_t addr) const {return &memory[addr];}
    uint8_t *GetBytePtr(uint32_t addr) {return &memory[addr];}

    void ClearMemory();

protected:
    std::array<uint8_t, 0xFFFFFF> memory; // This will probably be going away.
    std::array<uint8_t, 0x20000> wram; // 0x7E0000 - 0x7FFFFF

    // The following blocks are mirrored in each of the banks from 0x00-0x3F and 0x80-0xBF.
    std::array<uint8_t, 0xFF> ioPorts21; // 0x2100-0x21FF. Unused: 0x2184+
    std::array<uint8_t, 0xFF> ioPorts40; // 0x4000-0x40FF. Only 0x4016 and 0x4017 are used.
    std::array<uint8_t, 0xFF> ioPorts42; // 0x4200-0x42FF. Unused: 0x420E,0x420F,0x4220-42FF
    std::array<uint8_t, 0xFF> ioPorts43; // 0x4300-0x43FF. 0x43[0-7][0-B] are used, the rest are unused.
    std::array<uint8_t, 0x2000> expansion; // 0x6000-0x7FFF.

    std::vector<uint8_t> rom;

    InfoInterface *infoInterface;
    Timer *timer;

    friend class MemoryTest;
};