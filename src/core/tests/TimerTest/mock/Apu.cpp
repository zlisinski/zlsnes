#include "Apu.h"
#include "Timer.h"


Apu::Apu(IoRegisterSubject *io, Timer *timer)
{
    (void)io;
    (void)timer;
}


Apu::~Apu()
{

}


void Apu::Step(uint32_t clocks)
{
    (void)clocks;
}


uint8_t Apu::ReadRegister(EIORegisters ioReg)
{
    (void)ioReg;
    return 0;
}


bool Apu::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    (void)ioReg;
    (void)byte;
    return true;
}