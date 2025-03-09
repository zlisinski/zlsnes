#ifndef ZLSNES_CORE_APU_H
#define ZLSNES_CORE_APU_H

#include "Zlsnes.h"
#include "IoRegisterProxy.h"


class Timer;


class Apu : public IoRegisterProxy
{
public:
    Apu(IoRegisterSubject *io, Timer *timer);
    virtual ~Apu();

    void Step(uint32_t clocks = 1);

private:
    uint8_t ReadRegister(EIORegisters ioReg) override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;
};

#endif