#include "Memory.h"

Memory::Memory()
{

}


Memory::~Memory()
{

}


void Memory::SetRomMemory(std::vector<uint8_t> &bootRomMemory, std::vector<uint8_t> &gameRomMemory)
{
    (void)bootRomMemory;
    (void)gameRomMemory;
}


void Memory::SetRomMemory(std::vector<uint8_t> &gameRomMemory)
{
    (void)gameRomMemory;
}


uint8_t Memory::Read8Bit(uint32_t addr) const
{
    //printf("Read %08X\n", addr);
    return memory[addr];
}


void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    memory[addr] = value;
}