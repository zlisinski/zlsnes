#ifndef ZLSNES_CORE_APU_H
#define ZLSNES_CORE_APU_H

#include "Zlsnes.h"
#include "IoRegisterProxy.h"


class Timer;

namespace Audio
{
    class Spc700;
    class Memory;
    class Timer;
}


class Apu : public IoRegisterProxy
{
public:
    Apu(IoRegisterSubject *io, Timer *timer);
    virtual ~Apu();

    void Step(uint32_t clocks = 1);

private:
    uint8_t ReadRegister(EIORegisters ioReg) override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    Audio::Memory *apuMemory;
    Audio::Timer *apuTimer;
    Audio::Spc700 *apuCpu;

    uint8_t &regAPUIO0; // 0x2140 Main CPU to Sound CPU Communication Port 0
    uint8_t &regAPUIO1; // 0x2141 Main CPU to Sound CPU Communication Port 1
    uint8_t &regAPUIO2; // 0x2142 Main CPU to Sound CPU Communication Port 2
    uint8_t &regAPUIO3; // 0x2143 Main CPU to Sound CPU Communication Port 3

    uint8_t &regCPUIO0; // CPU Input and Output Register 0 (R and W)
    uint8_t &regCPUIO1; // CPU Input and Output Register 1 (R and W)
    uint8_t &regCPUIO2; // CPU Input and Output Register 2 (R and W)
    uint8_t &regCPUIO3; // CPU Input and Output Register 3 (R and W)
};

#endif