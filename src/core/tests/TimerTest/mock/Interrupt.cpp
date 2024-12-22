#include "Interrupt.h"


Interrupt::Interrupt() :
    isNmi(false),
    isIrq(false)
{

}


void Interrupt::RequestNmi()
{
    LogInterrupt("Request VBlank NMI");
    isNmi = true;
}


void Interrupt::ClearNmi()
{
    LogInterrupt("ClearNmi");
    isNmi = false;
}


void Interrupt::RequestIrq()
{
    LogInterrupt("Request IRQ");
    isIrq = true;
}


void Interrupt::ClearIrq()
{
    LogInterrupt("ClearIrq");
    isIrq = false;
}