#include "Memory.h"

Memory::Memory(InfoInterface *infoInterface)
{
    (void)infoInterface;
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

uint8_t Memory::Read8Bit(uint32_t addr) const
{
    return memory[addr];
}

void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    memory[addr] = value;
}

void Memory::ClearMemory()
{
    memory.fill(0);
}