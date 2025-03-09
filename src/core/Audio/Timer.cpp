#include "Timer.h"
#include "Memory.h"


namespace Audio
{


Timer::Timer(Memory *memory) :
    memory(memory)
{

}

Timer::~Timer()
{

}


void Timer::AddCycle(uint8_t cycles)
{
    clockCounter += cycles;
}


}