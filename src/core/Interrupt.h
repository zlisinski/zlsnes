#ifndef ZLSNES_CORE_INTERRUPT_H
#define ZLSNES_CORE_INTERRUPT_H

#include "Zlsnes.h"

class Interrupt
{
public:
    Interrupt();
    virtual ~Interrupt() {}

    void RequestNmi() {isNmi = true; LogInterrupt("Request VBlank NMI");}
    void RequestIrq() {isIrq = true; LogInterrupt("Request IRQ");}
    void ClearNmi() {isNmi = false;}
    void ClearIrq() {isIrq = false;}

    bool CheckInterrupts() const {return isNmi || isIrq;}
    bool IsNmi() const {return isNmi;}
    bool IsIrq() const {return isIrq;}

protected:
    bool isNmi;
    bool isIrq;
};

#endif