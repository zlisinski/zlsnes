#ifndef ZLSNES_CORE_TIMER_H
#define ZLSNES_CORE_TIMER_H

#include "Zlsnes.h"
#include "TimerObserver.h"

enum EClockSpeed
{
    eClockFastRom = 6,
    eClockIoReg = 6,
    eClockInternal = 6,
    eClockSlowRom = 8,
    eClockWRam = 8,
    eClockOther = 12
};

class Timer : public TimerSubject
{
public:
    Timer();
    virtual ~Timer() {}

    void AddCycle(uint8_t clocks);

private:
    uint32_t internalCounter;

    friend class AddressModeTest;
};

#endif