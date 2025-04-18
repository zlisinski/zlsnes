#ifndef ZLSNES_CORE_MEMORY_H
#define ZLSNES_CORE_MEMORY_H

#include <array>

#include "Address.h"
#include "Bytes.h"
#include "IoRegisterProxy.h"
#include "Zlsnes.h"


class Cartridge;
class DebuggerInterface;
class InfoInterface;
class Timer;
class Ppu;


class Memory : public IoRegisterSubject
{
public:
    Memory(InfoInterface *infoInterface = nullptr, DebuggerInterface *debuggerInterface = nullptr);
    virtual ~Memory();

    void SetCartridge(Cartridge *cart) {(void)cart;}
    void SetTimer(Timer *timer) {(void)timer;}
    void SetPpu(Ppu *ppu) {(void)ppu;}

    template<bool addTime = true>
    uint8_t Read8Bit(uint32_t addr);
    template<bool addTime = true>
    uint8_t Read8Bit(const Address &addr)
    {
        return Read8Bit<addTime>(addr.ToUint());
    }
    // Bypasses special read code. Only use for Debugger.
    uint8_t ReadRaw8Bit(uint32_t addr) //const
    {
        return *GetBytePtr(addr);
    }

    // Reads that don't wrap at bank boundaries. E.G. Read16Bit(0x12FFFF) will read from 0x12FFFF and 0x130000.
    template<bool addTime = true>
    uint16_t Read16Bit(uint32_t addr)
    {
        return Bytes::Make16Bit(Read8Bit<addTime>(addr + 1), Read8Bit<addTime>(addr));
    }
    template<bool addTime = true>
    uint16_t Read16Bit(const Address &addr)
    {
        uint8_t low = Read8Bit<addTime>(addr);
        uint8_t high = Read8Bit<addTime>(addr.AddOffset(1));
        return Bytes::Make16Bit(high, low);
    }
    template<bool addTime = true>
    uint32_t Read24Bit(uint32_t addr)
    {
        uint8_t low = Read8Bit<addTime>(addr);
        uint8_t mid = Read8Bit<addTime>(addr + 1);
        uint8_t high = Read8Bit<addTime>(addr + 2);
        return Bytes::Make24Bit(high, mid, low);
    }
    template<bool addTime = true>
    uint32_t Read24Bit(const Address &addr)
    {
        uint8_t low = Read8Bit<addTime>(addr);
        uint8_t mid = Read8Bit<addTime>(addr.AddOffset(1));
        uint8_t high = Read8Bit<addTime>(addr.AddOffset(2));
        return Bytes::Make24Bit(high, mid, low);
    }

    // Reads that wrap at bank boundaries. E.G. Read16BitWrapBank(0x12, 0xFFFF) will read from 0x12FFFF and 0x120000.
    template<bool addTime = true>
    uint16_t Read16BitWrapBank(uint8_t bank, uint16_t addr)
    {
        uint8_t low = Read8Bit<addTime>(Bytes::Make24Bit(bank, addr));
        uint8_t high = Read8Bit<addTime>(Bytes::Make24Bit(bank, addr + 1));
        return Bytes::Make16Bit(high, low);
    }
    template<bool addTime = true>
    uint16_t Read16BitWrapBank(const Address &addr)
    {
        uint8_t low = Read8Bit<addTime>(addr);
        uint8_t high = Read8Bit<addTime>(addr.AddOffsetWrapBank(1));
        return Bytes::Make16Bit(high, low);
    }
    template<bool addTime = true>
    uint32_t Read24BitWrapBank(uint8_t bank, uint16_t addr)
    {
        uint8_t low = Read8Bit<addTime>(Bytes::Make24Bit(bank, addr));
        uint8_t mid = Read8Bit<addTime>(Bytes::Make24Bit(bank, addr + 1));
        uint8_t high = Read8Bit<addTime>(Bytes::Make24Bit(bank, addr + 2));
        return Bytes::Make24Bit(high, mid, low);
    }
    template<bool addTime = true>
    uint32_t Read24BitWrapBank(const Address &addr)
    {
        uint8_t low = Read8Bit<addTime>(addr);
        uint8_t mid = Read8Bit<addTime>(addr.AddOffsetWrapBank(1));
        uint8_t high = Read8Bit<addTime>(addr.AddOffsetWrapBank(2));
        return Bytes::Make24Bit(high, mid, low);
    }

    template<bool addTime = true>
    void Write8Bit(uint32_t addr, uint8_t value);
    template<bool addTime = true>
    void Write8Bit(const Address &addr, uint8_t value)
    {
        Write8Bit<addTime>(addr.ToUint(), value);
    }
    template<bool addTime = true>
    void Write16Bit(const Address &addr, uint16_t value)
    {
        Write8Bit<addTime>(addr.ToUint(), value & 0xFF);
        Write8Bit<addTime>(addr.AddOffset(1).ToUint(), value >> 8);
    }
    template<bool addTime = true>
    void Write16BitWrapBank(const Address &addr, uint16_t value)
    {
        Write8Bit<addTime>(addr.ToUint(), value & 0xFF);
        Write8Bit<addTime>(addr.AddOffsetWrapBank(1).ToUint(), value >> 8);
    }

    // Bypasses checking of reads/writes from/to special addresses. Don't use unless you know what you are doing.
    //const uint8_t *GetBytePtr(uint32_t addr) const {return &memory[addr];}
    uint8_t *GetBytePtr(uint32_t addr);// {return &memory[addr];}

    void ClearMemory();

protected:
    // Inherited from IoRegisterSubject.
    uint8_t &GetIoRegisterRef(EIORegisters ioReg) override;
    
    std::array<uint8_t, 0xFFFFFF> memory;
};

#endif