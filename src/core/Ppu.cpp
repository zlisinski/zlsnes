#include <sstream>

#include "IoRegisters.h"
#include "Memory.h"
#include "Ppu.h"


Ppu::Ppu(Memory *memory) :
    memory(memory)
    //regINIDISP(memory->AttachIoRegister(eRegINIDISP, this))
{

}

bool Ppu::WriteByte(uint32_t address, uint8_t byte)
{
    LogInstruction("Display::WriteByte %06X, %02X", address, byte);

    /*switch (address)
    {
        case eRegINIDISP:
            *regINIDISP = byte;
            return true;
        default:
            return false;
    }*/
    return false;
}

uint8_t Ppu::ReadByte(uint32_t address) const
{
    LogInstruction("Display::ReadByte %06X", address);

    /**switch (address)
    {
        case eRegINIDISP:
            return *regINIDISP;
        default:
            std::stringstream ss;
            ss << "Display doesnt handle reads to 0x" << std::hex << std::setw(6) << std::setfill('0') << address;
            throw std::range_error(ss.str());
    }*/
    return 0;
}