#include "Apu.h"
#include "Timer.h"

#include "Audio/Memory.h"
#include "Audio/Spc700.h"
#include "Audio/Timer.h"


Apu::Apu(IoRegisterSubject *io, Timer *timer) :
    apuMemory(new Audio::Memory),
    apuTimer(new Audio::Timer(apuMemory)),
    apuCpu(new Audio::Spc700(apuMemory, apuTimer)),
    regAPUIO0(io->RequestOwnership(eRegAPUIO0, this)),
    regAPUIO1(io->RequestOwnership(eRegAPUIO1, this)),
    regAPUIO2(io->RequestOwnership(eRegAPUIO2, this)),
    regAPUIO3(io->RequestOwnership(eRegAPUIO3, this)),
    regCPUIO0(apuMemory->RequestOwnership(eRegCPUIO0, this)),
    regCPUIO1(apuMemory->RequestOwnership(eRegCPUIO1, this)),
    regCPUIO2(apuMemory->RequestOwnership(eRegCPUIO2, this)),
    regCPUIO3(apuMemory->RequestOwnership(eRegCPUIO3, this))
{
    apuMemory->SetTimer(apuTimer);

    timer->SetApu(this);
}


Apu::~Apu()
{
    delete apuCpu;
    delete apuTimer;
    delete apuMemory;
}


void Apu::Step(uint32_t clocks)
{
    apuCpu->Step(clocks);
}


uint8_t Apu::ReadRegister(EIORegisters ioReg)
{
    LogApu("Apu::ReadRegister %04X", ioReg);

    switch(ioReg)
    {
        // Reads from APU registers return the value of the CPU register, and vice versa.
        case eRegAPUIO0:
            return regCPUIO0;
        case eRegAPUIO1:
            return regCPUIO1;
        case eRegAPUIO2:
            return regCPUIO2;
        case eRegAPUIO3:
            return regCPUIO3;
        /*case eRegCPUIO0:
            return regAPUIO0;
        case eRegCPUIO1:
            return regAPUIO1;
        case eRegCPUIO2:
            return regAPUIO2;
        case eRegCPUIO3:
            return regAPUIO3;*/
        default:
            throw std::range_error(fmt("Apu doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Apu::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogApu("Apu::WriteRegister %04X %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegAPUIO0:
            regAPUIO0 = byte;
            apuMemory->WriteIoPort(0, byte);
            return true;
        case eRegAPUIO1:
            regAPUIO1 = byte;
            apuMemory->WriteIoPort(1, byte);
            return true;
        case eRegAPUIO2:
            regAPUIO2 = byte;
            apuMemory->WriteIoPort(2, byte);
            return true;
        case eRegAPUIO3:
            regAPUIO3 = byte;
            apuMemory->WriteIoPort(3, byte);
            return true;
        case eRegCPUIO0:
            regCPUIO0 = byte;
            return true;
        case eRegCPUIO1:
            regCPUIO1 = byte;
            return true;
        case eRegCPUIO2:
            regCPUIO2 = byte;
            return true;
        case eRegCPUIO3:
            regCPUIO3 = byte;
            return true;
        default:
            throw std::range_error(fmt("Apu doesnt handle writes to 0x%04X", ioReg));
    }

    return true;
}