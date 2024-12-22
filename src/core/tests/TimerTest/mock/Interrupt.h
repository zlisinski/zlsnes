#ifndef ZLSNES_CORE_INTERRUPT_H
#define ZLSNES_CORE_INTERRUPT_H

#include "Zlsnes.h"

class Interrupt
{
public:
    Interrupt();
    virtual ~Interrupt() {}

    void RequestNmi();
    void ClearNmi();

    void RequestIrq();
    void ClearIrq();

    bool CheckInterrupts() const {return isNmi || isIrq;}
    bool IsNmi() const {return isNmi;}
    bool IsIrq() const {return isIrq;}

protected:
    bool isNmi;
    bool isIrq;
};

#endif