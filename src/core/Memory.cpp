#include "Memory.h"
#include "Cartridge.h"
#include "Timer.h"


Memory::Memory(InfoInterface *infoInterface) :
    memory(),
    infoInterface(infoInterface),
    timer(NULL)
{

}


Memory::~Memory()
{

}


void Memory::SetRomMemory(std::vector<uint8_t> &gameRomMemory)
{
    (void)gameRomMemory;
}


void Memory::SetTimer(Timer *timer)
{
    this->timer = timer;
}


uint8_t Memory::Read8Bit(uint32_t addr) const
{
    // TODO: Figure out cycles to add based on location being read and ROM type.
    timer->AddCycle(EClockSpeed::eClockFastRom);

    return memory[addr];
}


void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    // TODO: Figure out cycles to add based on location being written and ROM type.
    timer->AddCycle(EClockSpeed::eClockFastRom);

    memory[addr] = value;
}