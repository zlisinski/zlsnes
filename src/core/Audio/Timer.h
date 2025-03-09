#ifndef ZLSNES_CORE_APU_TIMER_H
#define ZLSNES_CORE_APU_TIMER_H

#include "../Zlsnes.h"

namespace Audio
{


class Memory;


class Timer
{
public:
    Timer(Memory *memory);
    ~Timer();

    inline uint32_t GetCounter() {return clockCounter;}
    inline void ResetCounter() {clockCounter = 0;}

    void AddCycle(uint8_t cycles = 1);

private:
    uint32_t clockCounter = 0;

    Memory *memory = nullptr;
};


} // end namespace

#endif