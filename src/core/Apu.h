#ifndef ZLSNES_CORE_APU_H
#define ZLSNES_CORE_APU_H

#include "Zlsnes.h"
#include "IoRegisterProxy.h"


class Apu : public IoRegisterProxy
{
public:
    Apu(IoRegisterSubject *memory);
    virtual ~Apu() {}

private:
    uint8_t ReadRegister(EIORegisters ioReg) override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    uint8_t &regAPUI00; // 0x2140 Main CPU to Sound CPU Communication Port 0
    uint8_t &regAPUI01; // 0x2141 Main CPU to Sound CPU Communication Port 1
    uint8_t &regAPUI02; // 0x2142 Main CPU to Sound CPU Communication Port 2
    uint8_t &regAPUI03; // 0x2143 Main CPU to Sound CPU Communication Port 3

    bool isInit;
};

#endif