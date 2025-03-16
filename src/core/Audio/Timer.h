#ifndef ZLSNES_CORE_APU_TIMER_H
#define ZLSNES_CORE_APU_TIMER_H

#include "../Zlsnes.h"
#include "../IoRegisterProxy.h"


namespace Audio
{


class Memory;


class Timer : public IoRegisterProxy
{
public:
    Timer(Memory *memory);
    virtual ~Timer();

    inline uint32_t GetCounter() {return clockCounter;}
    inline void ResetCounter() {clockCounter = 0;}

    void AddCycle(uint8_t cycles = 1);

    void EnableTimer0(bool value);
    void EnableTimer1(bool value);
    void EnableTimer2(bool value);

private:
    // Inherited from IoRegisterProxy.
    uint8_t ReadRegister(EIORegisters ioReg) override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    uint32_t clockCounter = 0;
    uint32_t counter8k = 0;
    uint32_t counter64k = 0;

    bool isTimerEnabled[3] = {false, false, false};
    uint8_t timerVal[3] = {0, 0, 0};
    uint8_t timerDiv[3] = {0, 0, 0};

    Memory *memory = nullptr;

    uint8_t &regT0DIV; // 0xFA Timer 0 Divider (for 8000Hz clock source) (W)
    uint8_t &regT1DIV; // 0xFB Timer 1 Divider (for 8000Hz clock source) (W)
    uint8_t &regT2DIV; // 0xFC Timer 2 Divider (for 64000Hz clock source) (W)
    uint8_t &regT0OUT; // 0xFD Timer 0 Output (R)
    uint8_t &regT1OUT; // 0xFE Timer 1 Output (R)
    uint8_t &regT2OUT; // 0xFF Timer 2 Output (R)
};


} // end namespace

#endif