#include "Memory.h"

namespace Audio
{


Memory::Memory()
{

}


Memory::~Memory()
{

}


uint8_t Memory::Read8Bit(uint16_t addr)
{
    return ram[addr];
}


void Memory::Write8Bit(uint16_t addr, uint8_t value)
{
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