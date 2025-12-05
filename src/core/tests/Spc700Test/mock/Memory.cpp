#include "Memory.h"
#include "Timer.h"

namespace Audio
{


Memory::Memory()
{

}


Memory::~Memory()
{

}


void Memory::WriteIoPort(uint8_t port, uint8_t byte)
{
    cpuReadPorts[port] = byte;
}


uint8_t Memory::Read8Bit(uint16_t addr)
{
    timer->AddCycle();
    return ram[addr];
}


void Memory::Write8Bit(uint16_t addr, uint8_t value)
{
    timer->AddCycle();
    ram[addr] = value;
}


void Memory::ClearMemory()
{
    ram.fill(0);
}


uint8_t *Memory::GetBytePtr(uint32_t addr)
{
    return &ram[addr];
}


uint8_t &Memory::GetIoRegisterRef(EIORegisters ioReg)
{
    return ram[ioReg];
}


}