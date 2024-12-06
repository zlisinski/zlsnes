#include "Memory.h"
#include "Cartridge.h"
#include "Timer.h"

static const uint32_t WRAM_OFFSET = 0x7E0000;
static const uint32_t WRAM_SIZE = 0x20000;

Memory::Memory(InfoInterface *infoInterface) :
    memory(),
    wram(),
    ioPorts21(),
    ioPorts40(),
    ioPorts42(),
    ioPorts43(),
    expansion(),
    infoInterface(infoInterface),
    timer(NULL)
{

}


Memory::~Memory()
{

}


void Memory::SetRomMemory(std::vector<uint8_t> &gameRomMemory)
{
    rom = gameRomMemory;
}


void Memory::SetTimer(Timer *timer)
{
    this->timer = timer;
}


uint8_t Memory::Read8Bit(uint32_t addr) const
{
    if ((addr & 0x408000) == 0) // Bank is in range 0x00-0x3F or 0x80-0xBF, and offset is in range 0x0000-0x7FFF.
    {
        uint8_t page = Bytes::GetByte<1>(addr);
        switch (page)
        {
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
                timer->AddCycle(EClockSpeed::eClockWRam);
                return wram[addr & 0x1FFF];
            case 0x21:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                return ioPorts21[addr & 0xFF];
            case 0x40:
                timer->AddCycle(EClockSpeed::eClockOther);
                return ioPorts40[addr & 0xFF];
            case 0x42:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                return ioPorts42[addr & 0xFF];
            case 0x43:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                return ioPorts43[addr & 0xFF];
            default:
                LogError("Read from unhandled address %06X", addr);
                return 0;
        }
    }

    if ((addr & 0xFE0000) == 0x7E0000) // Bank is 0x7E or 0x7F.
    {
        timer->AddCycle(EClockSpeed::eClockWRam);
        return wram[addr & 0x1FFFF];
    }

    if (addr & 0x8000)
    {
        // TODO: Figure out if this is slow or fast
        timer->AddCycle(EClockSpeed::eClockFastRom);

        // Assume LoROM for now.
        // Remove the high bit of the offset and shift the bank right one so that LSBit of bank is MSBit of offset.
        uint32_t mappedAddr = (((addr & 0xFF0000) >> 1) | (addr & 0x7FFF));
        return rom[mappedAddr];
    }

    // TODO: Figure out if this is slow or fast
    timer->AddCycle(EClockSpeed::eClockFastRom);

    LogError("Read from HiROM area %06X", addr);
    return 0;
}


void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    if ((addr & 0x408000) == 0) // Bank is in range 0x00-0x3F or 0x80-0xBF, and offset is in range 0x0000-0x7FFF.
    {
        uint8_t page = Bytes::GetByte<1>(addr);
        switch (page)
        {
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
                timer->AddCycle(EClockSpeed::eClockWRam);
                wram[addr & 0x1FFF] = value;
                return;
            case 0x21:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                ioPorts21[addr & 0xFF] = value;
                return;
            case 0x40:
                timer->AddCycle(EClockSpeed::eClockOther);
                ioPorts40[addr & 0xFF] = value;
                return;
            case 0x42:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                ioPorts42[addr & 0xFF] = value;
                return;
            case 0x43:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                ioPorts43[addr & 0xFF] = value;
                return;
            default:
                LogError("Write to unhandled address %06X", addr);
                printf("Write to unhandled address %06X\n", addr);
                return;
        }
    }

    if ((addr & 0xFE0000) == 0x7E0000) // Bank is 0x7E or 0x7F.
    {
        timer->AddCycle(EClockSpeed::eClockWRam);
        wram[addr & 0x1FFFF] = value;
        return;
    }

    if (addr & 0x8000)
    {
        // TODO: Figure out if this is slow or fast
        timer->AddCycle(EClockSpeed::eClockFastRom);

        // Assume LoROM for now.
        // Remove the high bit of the offset and shift the bank right one so that LSBit of bank is MSBit of offset.
        uint32_t mappedAddr = (((addr & 0xFF0000) >> 1) | (addr & 0x7FFF));
        LogError("Attempting to write to ROM at %06X", mappedAddr);
        return;
    }

    // TODO: Figure out if this is slow or fast
    timer->AddCycle(EClockSpeed::eClockFastRom);

    LogError("Write to HiROM area %06X", addr);
    return;
}


void Memory::ClearMemory()
{
    memory.fill(0);
    wram.fill(0);
    ioPorts21.fill(0);
    ioPorts40.fill(0);
    ioPorts42.fill(0);
    ioPorts43.fill(0);
    expansion.fill(0);
}