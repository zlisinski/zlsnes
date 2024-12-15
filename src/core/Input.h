#pragma once

#include "Zlsnes.h"
#include "Buttons.h"
//#include "Interrupt.h"
#include "IoRegisterProxy.h"

class Input : public IoRegisterProxy
{
public:
    Input(IoRegisterSubject *ioRegisterSubject/*, Interrupt *interrupts*/);
    virtual ~Input();

    void SetButtons(const Buttons &buttons);

    bool SaveState(FILE *file);
    bool LoadState(uint16_t version, FILE *file);

    // Inherited from IoRegisterProxy.
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;
    uint8_t ReadRegister(EIORegisters ioReg) const override;

private:
    //uint8_t *regP1;
    //Interrupt *interrupts;

    Buttons buttonData;
};