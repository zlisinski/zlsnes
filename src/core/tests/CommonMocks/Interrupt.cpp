#include "Interrupt.h"


Interrupt::Interrupt() :
    isNmi(false),
    isIrq(false)
{

}


void Interrupt::RequestNmi()
{
    isNmi = true;
}


void Interrupt::ClearNmi()
{
    isNmi = false;
}


void Interrupt::RequestIrq()
{
    isIrq = true;
}


void Interrupt::ClearIrq()
{
    isIrq = false;
}