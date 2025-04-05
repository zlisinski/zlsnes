#include "Timer.h"
#include "Memory.h"


namespace Audio
{


Timer::Timer(Memory *memory) :
    memory(memory),
    regT0DIV(memory->RequestOwnership(eRegT0DIV, this)),
    regT1DIV(memory->RequestOwnership(eRegT1DIV, this)),
    regT2DIV(memory->RequestOwnership(eRegT2DIV, this)),
    regT0OUT(memory->RequestOwnership(eRegT0OUT, this)),
    regT1OUT(memory->RequestOwnership(eRegT1OUT, this)),
    regT2OUT(memory->RequestOwnership(eRegT2OUT, this))
{

}


Timer::~Timer()
{

}


void Timer::AddCycle(uint8_t cycles)
{
    clockCounter += cycles;
    counter8k += cycles;
    counter64k += cycles;
    LogSpcTimer("Timer::AddCycle cycles=%02X counter8k=%d counter64k=%d", cycles, counter8k, counter64k);

    if (counter8k >= 128)
    {
        counter8k -= 128;

        timerVal[0]++;
        if (isTimerEnabled[0] && timerVal[0] >= timerDiv[0])
        {
            timerVal[0] = 0;
            regT0OUT = (regT0OUT + 1) & 0x0F;
        }

        timerVal[1]++;
        if (isTimerEnabled[1] && timerVal[1] >= timerDiv[1])
        {
            timerVal[1] = 0;
            regT1OUT = (regT1OUT + 1) & 0x0F;
        }
    }

    if (counter64k >= 16)
    {
        counter64k -= 16;

        timerVal[2]++;
        if (isTimerEnabled[2] && timerVal[2] >= timerDiv[2])
        {
            timerVal[2] = 0;
            regT2OUT = (regT2OUT + 1) & 0x0F;
        }
    }
}


void Timer::EnableTimer0(bool value)
{
    if (!isTimerEnabled[0] && value)
    {
        timerVal[0] = 0;
        regT0OUT = 0;
    }

    isTimerEnabled[0] = value;
}


void Timer::EnableTimer1(bool value)
{
    if (!isTimerEnabled[1] && value)
    {
        timerVal[1] = 0;
        regT1OUT = 0;
    }

    isTimerEnabled[1] = value;
}


void Timer::EnableTimer2(bool value)
{
    if (!isTimerEnabled[2] && value)
    {
        timerVal[2] = 0;
        regT2OUT = 0;
    }

    isTimerEnabled[2] = value;
}


uint8_t Timer::ReadRegister(EIORegisters ioReg)
{
    switch (ioReg)
    {
        case eRegT0OUT: // 0xFD
        {
            uint8_t value = regT0OUT;
            regT0OUT = 0;
            return value;
        }

        case eRegT1OUT: // 0xFE
        {
            uint8_t value = regT1OUT;
            regT1OUT = 0;
            return value;
        }

        case eRegT2OUT: // 0xFF
        {
            uint8_t value = regT2OUT;
            regT2OUT = 0;
            return value;
        }

        default:
            throw std::range_error(fmt("Timer doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Timer::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    switch (ioReg)
    {
        case eRegT0DIV: // 0xFA
        case eRegT1DIV: // 0xFB
        case eRegT2DIV: // 0xFC
            timerDiv[ioReg - eRegT0DIV] = byte;
            LogSpcTimer("T%dDIV=%02X", ioReg - eRegT0DIV, byte);
            return true;

        default:
            throw std::range_error(fmt("Timer doesnt handle writes to 0x%04X", ioReg));
    }
}


}