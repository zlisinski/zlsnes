#include "Apu.h"

Apu::Apu(IoRegisterSubject *memory) :
    regAPUI00(memory->RequestOwnership(eRegAPUI00, this)),
    regAPUI01(memory->RequestOwnership(eRegAPUI01, this)),
    regAPUI02(memory->RequestOwnership(eRegAPUI02, this)),
    regAPUI03(memory->RequestOwnership(eRegAPUI03, this)),
    isInit(false)
{

}


uint8_t Apu::ReadRegister(EIORegisters ioReg)
{
    LogApu("Apu::ReadRegister %04X", ioReg);

    switch(ioReg)
    {
        case eRegAPUI00:
            if (isInit)
                return regAPUI00;
            else
                return 0xAA;
        case eRegAPUI01:
            if (isInit)
                return regAPUI01;
            else
                return 0xBB;
        case eRegAPUI02:
            return regAPUI02;
        case eRegAPUI03:
            return regAPUI03;
        default:
            throw std::range_error(fmt("Apu doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Apu::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogApu("Apu::WriteRegister %04X %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegAPUI00:
            if (byte == 0xCC && !isInit)
            {
                LogApu("Go to stage2");
                isInit = true;
            }
            regAPUI00 = byte;
            return true;
        case eRegAPUI01:
            if (byte == 0xFF && regAPUI00 == 0x00 && regAPUI01 == 0x00 && regAPUI02 == 0x00 && regAPUI03 == 0x00)
            {
                LogApu("Go to stage1");
                isInit = false;
            }
            regAPUI01 = byte;
            return true;
        case eRegAPUI02:
            regAPUI02 = byte;
            return true;
        case eRegAPUI03:
            regAPUI03 = byte;
            return true;
        default:
            throw std::range_error(fmt("Apu doesnt handle writes to 0x%04X", ioReg));
    }

    return true;
}