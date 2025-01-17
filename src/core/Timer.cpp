#include "Timer.h"
#include "Interrupt.h"
#include "Memory.h"


const uint32_t CLOCKS_PER_H = 4;
const uint32_t CLOCKS_PER_V = 1364;
const uint32_t H_PER_V = CLOCKS_PER_V / CLOCKS_PER_H; // 341


Timer::Timer(Memory *memory, Interrupt *interrupts) :
    clockCounter(0),
    hCount(0),
    vCount(0),
    isHBlank(true),
    isVBlank(false),
    irqTrigger(0),
    hTrigger(0x1FF),
    vTrigger(0x1FF),
    memory(memory),
    interrupts(interrupts),
    regNMITIMEN(memory->RequestOwnership(eRegNMITIMEN, this)),
    regHTIMEL(memory->RequestOwnership(eRegHTIMEL, this)),
    regHTIMEH(memory->RequestOwnership(eRegHTIMEH, this)),
    regVTIMEL(memory->RequestOwnership(eRegVTIMEL, this)),
    regVTIMEH(memory->RequestOwnership(eRegVTIMEH, this)),
    regRDNMI(memory->RequestOwnership(eRegRDNMI, this)),
    regTIMEUP(memory->RequestOwnership(eRegTIMEUP, this)),
    regHVBJOY(memory->RequestOwnership(eRegHVBJOY, this))
{
    regHTIMEH = 0x01;
    regHTIMEL = 0xFF;
    regVTIMEH = 0x01;
    regVTIMEL = 0xFF;
}


void Timer::AddCycle(uint8_t cycles)
{
    uint16_t oldHCount = hCount;

    // Since a single call to this function can advance hCount by more than 1, we have to check a range of values,
    // including rolling over from 341 to 0.
    #define HCOUNT_GE(val) (hCount >= (val) && (oldHCount < (val) || oldHCount > hCount))

    clockCounter += cycles;
    hCount = clockCounter / CLOCKS_PER_H;

    // Note: HBlankEnd for a scanline comes before HBlankStart for the same scanline.

    if (HCOUNT_GE(1))
    {
        // If we just rolled over.
        ProcessHBlankEnd();
    }
    else if (hCount >= 134 && oldHCount < 134)
    {
        // DRAM refresh.
        cycles += 40; // For NotifyTimerObservers();
        clockCounter += 40;
        hCount = clockCounter / CLOCKS_PER_H;
    }
    else if (hCount >= 274 && oldHCount < 274)
    {
        // We just entered hblank.
        ProcessHBlankStart();
    }
    else if (clockCounter >= 1364)
    {
        clockCounter -= 1364;
        hCount = clockCounter / CLOCKS_PER_H;

        // TODO: Check for number of scanlines per screen in regSETINI.

        vCount++;
        if (vCount == 225)
        {
            ProcessVBlankStart();
        }
        else if (vCount == 228)
        {
            // If joypad auto read is enabled, toggle the busy flag.
            if (Bytes::GetBit<0>(regNMITIMEN))
                Bytes::ClearBit<0>(regHVBJOY);
        }
        else if (vCount == 262)
        {
            vCount = 0;
            ProcessVBlankEnd();
        }

        // We could have rolled over out of hblank, so check again.
        if (HCOUNT_GE(1))
        {
            ProcessHBlankEnd();
        }
    }

    if ((irqTrigger == 3 && vCount == vTrigger && HCOUNT_GE(hTrigger)) ||
        (irqTrigger == 2 && vCount == vTrigger && HCOUNT_GE(0)) ||
        (irqTrigger == 1 && HCOUNT_GE(hTrigger)))
    {
        interrupts->RequestIrq();
    }

    NotifyTimerObservers(cycles);
}


void Timer::ProcessHBlankStart()
{
    isHBlank = true;

    // Set HBlank flag.
    Bytes::ClearBit<6>(regHVBJOY);

    // Notify anyone who wants to know when HBlank starts.
    NotifyHBlankStartObservers(vCount);
}


void Timer::ProcessHBlankEnd()
{
    isHBlank = false;

    // Clear HBlank flag.
    Bytes::SetBit<6>(regHVBJOY);

    // Notify anyone who wants to know when HBlank ends.
    NotifyHBlankEndObservers(vCount);
}


void Timer::ProcessVBlankStart()
{
    isVBlank = true;

    // Set VBlank flags.
    Bytes::SetBit<7>(regRDNMI);
    Bytes::SetBit<7>(regHVBJOY);

    // Request VBlank interrupt if enabled.
    if (Bytes::GetBit<7>(regNMITIMEN))
        interrupts->RequestNmi();

    // If joypad auto read is enabled, toggle the busy flag.
    if (Bytes::GetBit<0>(regNMITIMEN))
        Bytes::SetBit<0>(regHVBJOY);

    // Notify anyone who wants to know when VBlank starts.
    NotifyVBlankStartObservers();
}


void Timer::ProcessVBlankEnd()
{
    isVBlank = false;

    // Clear VBlank flags.
    Bytes::ClearBit<7>(regRDNMI);
    Bytes::ClearBit<7>(regHVBJOY);

    // Notify anyone who wants to know when VBlank ends.
    NotifyVBlankEndObservers();
}


uint8_t Timer::ReadRegister(EIORegisters ioReg)
{
    LogTimer("Timer::ReadRegister %04X", ioReg);

    switch (ioReg)
    {
        case eRegNMITIMEN: // 0x4200
        case eRegHTIMEL: // 0x4207
        case eRegHTIMEH: // 0x4208
        case eRegVTIMEL: // 0x4209
        case eRegVTIMEH: // 0x420A
            return memory->GetOpenBusValue();

        case eRegRDNMI: // 0x4210
        {
            uint8_t value = regRDNMI;

            // VBlank NMI flag gets reset after reads.
            Bytes::ClearBit<7>(regRDNMI);

            // Bits 4-6 are open bus.
            value = (value & 0x8F) | (memory->GetOpenBusValue() & 0x70);

            return value;
        }

        case eRegTIMEUP: // 0x4211
        {
            uint8_t value = (regTIMEUP & 0x80) | (memory->GetOpenBusValue() & 0x7F);
            Bytes::ClearBit<7>(regTIMEUP);
            interrupts->ClearIrq();
            return value;
        }

        case eRegHVBJOY: // 0x4212
            return regHVBJOY;

        default:
            throw std::range_error(fmt("Timer doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Timer::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogTimer("Timer::WriteRegister %04X", ioReg);

    switch (ioReg)
    {
        case eRegNMITIMEN: // 0x4200
        {
            // Request VBlank interrupt if the enable flag changes while in VBlank.
            if (!Bytes::TestBit<7>(regNMITIMEN) && Bytes::TestBit<7>(byte) && Bytes::TestBit<7>(regRDNMI))
                interrupts->RequestNmi();

            regNMITIMEN = byte;
            irqTrigger = (byte >> 4) & 0x03;
            LogTimer("NMITIMEN=%02X irqTrigger=%02X", byte, irqTrigger);
            return true;
        }

        case eRegHTIMEL: // 0x4207
            regHTIMEL = byte;
            hTrigger = (hTrigger & 0xFF00) | byte;
            LogTimer("HTIMEL=%02X hTrigger=%04X", byte, hTrigger);
            return true;

        case eRegHTIMEH: // 0x4208
            regHTIMEH = byte;
            hTrigger = (byte << 8) | (hTrigger & 0xFF);
            LogTimer("HTIMEH=%02X hTrigger=%04X", byte, hTrigger);
            return true;

        case eRegVTIMEL: // 0x4209
            regVTIMEL = byte;
            vTrigger = (vTrigger & 0xFF00) | byte;
            LogTimer("VTIMEL=%02X vTrigger=%04X", byte, vTrigger);
            return true;

        case eRegVTIMEH: // 0x420A
            regVTIMEH = byte;
            vTrigger = (byte << 8) | (vTrigger & 0xFF);
            LogTimer("VTIMEH=%02X vTrigger=%04X", byte, vTrigger);
            return true;

        case eRegRDNMI: // 0x4210
        case eRegTIMEUP: // 0x4211
        case eRegHVBJOY: // 0x4212
        default:
            // Read-only or not Timer related address.
            throw std::range_error(fmt("Timer doesnt handle writes to 0x%04X", ioReg));
    }

    return false;
}