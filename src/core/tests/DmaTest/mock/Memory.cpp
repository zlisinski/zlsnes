#include "Memory.h"

// The linker needs this, otherwise the function definition needs to go in the header.
template uint8_t Memory::Read8Bit<true>(uint32_t addr);
template uint8_t Memory::Read8Bit<false>(uint32_t addr);
template void Memory::Write8Bit<true>(uint32_t addr, uint8_t value);
template void Memory::Write8Bit<false>(uint32_t addr, uint8_t value);

Memory::Memory(InfoInterface *infoInterface, DebuggerInterface *debuggerInterface)
{
    (void)infoInterface;
    (void)debuggerInterface;
}

Memory::~Memory()
{

}

template<bool addTime>
uint8_t Memory::Read8Bit(uint32_t addr)
{
    return memory[addr];
}

template<bool addTime>
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