#ifndef ZLSNES_CORE_APU_MEMORY
#define ZLSNES_CORE_APU_MEMORY

#include <array>

#include "Zlsnes.h"
#include "Bytes.h"
#include "IoRegisterProxy.h"


namespace Audio
{


class Timer;


class Memory : public IoRegisterSubject
{
public:
    Memory();
    virtual ~Memory();

    void SetTimer(Timer *timer) {this->timer = timer;}

    void WriteIoPort(uint8_t port, uint8_t byte);

    uint8_t Read8Bit(uint16_t addr);
    // Bypasses special read code. Only use for Debugger.
    uint8_t ReadRaw8Bit(uint16_t addr)
    {
        return ram[addr];
    }
    uint16_t Read16Bit(uint16_t addr)
    {
        return Bytes::Make16Bit(Read8Bit(addr + 1), Read8Bit(addr));
    }
    uint16_t Read16BitWrapPage(uint8_t page, uint8_t offset)
    {
        uint8_t low = Read8Bit(Bytes::Make16Bit(page, offset));
        uint8_t high = Read8Bit(Bytes::Make16Bit(page, offset + 1));
        return Bytes::Make16Bit(high, low);
    }
    uint16_t Read16BitWrapPage(uint16_t addr)
    {
        uint8_t low = Read8Bit(addr);
        uint8_t high = Read8Bit(Bytes::Make16Bit(Bytes::GetByte<1>(addr), Bytes::GetByte<0>(addr) + 1));
        return Bytes::Make16Bit(high, low);
    }

    void Write8Bit(uint16_t addr, uint8_t value);
    void Write16Bit(uint16_t addr, uint16_t value)
    {
        Write8Bit(addr, value & 0xFF);
        Write8Bit(addr + 1, value >> 8);
    }
    void Write16BitWrapPage(uint8_t page, uint8_t offset, uint16_t value)
    {
        Write8Bit(Bytes::Make16Bit(page, offset), value & 0xFF);
        Write8Bit(Bytes::Make16Bit(page, offset + 1), value >> 8);
    }
    void Write16BitWrapPage(uint16_t addr, uint16_t value)
    {
        Write8Bit(addr, value & 0xFF);
        Write8Bit(Bytes::Make16Bit(Bytes::GetByte<1>(addr), Bytes::GetByte<0>(addr) + 1), value >> 8);
    }

    // Bypasses checking of reads/writes from/to special addresses. Don't use unless you know what you are doing.
    uint8_t *GetBytePtr(uint16_t addr)
    {
        return &ram[addr];
    }

    void ClearMemory();

protected:
    uint8_t *GetBytePtr(uint32_t addr) override;
    uint8_t &GetIoRegisterRef(EIORegisters ioReg) override;

    std::array<uint8_t, 0x10000> ram = {0};
    uint8_t cpuReadPorts[4] = {0};

    bool bootRomEnabled = true;

    Timer *timer = nullptr;

    friend class MemoryTest;
};


}

#endif