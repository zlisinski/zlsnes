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

uint8_t *Memory::GetIoRegisterPtr(EIORegisters ioReg)
{
    return &memory[ioReg];
}