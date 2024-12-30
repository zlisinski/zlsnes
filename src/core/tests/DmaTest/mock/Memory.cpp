#include "Memory.h"

Memory::Memory(InfoInterface *infoInterface, DebuggerInterface *debuggerInterface)
{
    (void)infoInterface;
    (void)debuggerInterface;
}

Memory::~Memory()
{

}

void Memory::SetCartridge(Cartridge *cart)
{
    (void)cart;
}

void Memory::SetTimer(Timer *timer)
{
    (void)timer;
}

uint8_t Memory::Read8Bit(uint32_t addr)
{
    return memory[addr];
}

void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    // Let observers handle the update. If there are no observers for this address, continue with normal processing.
    if (WriteIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF), value))
    {
        return;
    }
    memory[addr] = value;
}

uint8_t *Memory::GetBytePtr(uint32_t addr)
{
    return &memory[addr];
}

void Memory::ClearMemory()
{
    memory.fill(0);
}

uint8_t &Memory::GetIoRegisterRef(EIORegisters ioReg)
{
    return memory[ioReg];
}