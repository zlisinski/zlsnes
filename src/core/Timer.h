#ifndef ZLSNES_CORE_TIMER_H
#define ZLSNES_CORE_TIMER_H

#include "Zlsnes.h"
#include "IoRegisterProxy.h"
#include "TimerObserver.h"

enum EClockSpeed
{
    eClockFastRom = 6,
    eClockIoReg = 6,
    eClockInternal = 6,
    eClockSlowRom = 8,
    eClockWRam = 8,
    eClockDma = 8,
    eClockOther = 12
};

class Apu;
class Interrupt;
class Memory;

class Timer : public TimerSubject, public IoRegisterProxy
{
public:
    Timer(Memory *memory, Interrupt *interrupts);
    virtual ~Timer() {}

    void SetApu(Apu *apu) {this->apu = apu;}

    void AddCycle(uint8_t cycles);

    inline uint16_t GetHCount() {return hCount;}
    inline uint16_t GetVCount() {return vCount;}
    inline bool GetIsHBlank() {return isHBlank;}
    inline bool GetIsVBlank() {return isVBlank;}

private:
    // Inherited from IoRegisterProxy.
    uint8_t ReadRegister(EIORegisters ioReg) override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    void ProcessHBlankStart();
    void ProcessHBlankEnd();
    void ProcessVBlankStart();
    void ProcessVBlankEnd();

    uint32_t clockCounter;
    uint32_t apuCounter;
    uint16_t hCount;
    uint16_t vCount;

    bool isHBlank;
    bool isVBlank;

    // NMITIMEN - 0x4200
    uint8_t irqTrigger;

    // [HV]TIME[HL] - 0x4207-0x420A
    uint16_t hTrigger;
    uint16_t vTrigger;

    Memory *memory;
    Interrupt *interrupts;
    Apu *apu;

    uint8_t &regNMITIMEN; // 0x4200
    uint8_t &regHTIMEL; // 0x4207
    uint8_t &regHTIMEH; // 0x4208
    uint8_t &regVTIMEL; // 0x4209
    uint8_t &regVTIMEH; // 0x420A
    uint8_t &regRDNMI; // 0x4210
    uint8_t &regTIMEUP; // 0x4211
    uint8_t &regHVBJOY; // 0x4212

    friend class TimerTest;
};

#endif