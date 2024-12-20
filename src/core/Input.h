#pragma once

#include "Zlsnes.h"
#include "Buttons.h"
//#include "Interrupt.h"
#include "IoRegisterProxy.h"
#include "TimerObserver.h"

class Memory;

class Input : public IoRegisterProxy, public TimerObserver
{
public:
    Input(Memory *memory, TimerSubject *timerSubject/*, Interrupt *interrupts*/);
    virtual ~Input();

    void SetButtons(const Buttons &buttons);

    bool SaveState(FILE *file);
    bool LoadState(uint16_t version, FILE *file);

    // Inherited from IoRegisterProxy.
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;
    uint8_t ReadRegister(EIORegisters ioReg) const override;

    // Inherited from TimerObserver.
    void UpdateTimer(uint32_t value) override;

private:
    Memory *memory;
    //Interrupt *interrupts;

    Buttons buttonData;

    bool lastAutoReadFlag;

    uint8_t &regJOY1L;
    uint8_t &regJOY1H;
    uint8_t &regJOY2L;
    uint8_t &regJOY2H;
    uint8_t &regJOY3L;
    uint8_t &regJOY3H;
    uint8_t &regJOY4L;
    uint8_t &regJOY4H;
};