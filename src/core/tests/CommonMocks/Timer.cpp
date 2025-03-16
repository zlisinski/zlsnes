#include "Timer.h"

Timer::Timer() :
    internalCounter(0)
{

}

void Timer::AddCycle(uint8_t clocks)
{
    internalCounter += clocks;
}