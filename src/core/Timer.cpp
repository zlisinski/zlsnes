#include "Timer.h"
#include "Memory.h"


const uint32_t CLOCKS_PER_H = 4;
const uint32_t CLOCKS_PER_V = 1364;
const uint32_t H_PER_V = CLOCKS_PER_V / CLOCKS_PER_H; // 341


Timer::Timer(Memory *memory) :
    clockCounter(0),
    hCount(0),
    vCount(0),
    isHBlank(true),
    isVBlank(false),
    memory(memory),
    regNMITIMEN(memory->RequestOwnership(eRegNMITIMEN, this)),
    regHTIMEL(memory->RequestOwnership(eRegHTIMEL, this)),
    regHTIMEH(memory->RequestOwnership(eRegHTIMEH, this)),
    regVTIMEL(memory->RequestOwnership(eRegVTIMEL, this)),
    regVTIMEH(memory->RequestOwnership(eRegVTIMEH, this)),
    regRDNMI(memory->RequestOwnership(eRegRDNMI, this)),
    regTIMEUP(memory->RequestOwnership(eRegTIMEUP, this)),
    regHVBJOY(memory->RequestOwnership(eRegHVBJOY, this))
{
    
}


void Timer::AddCycle(uint8_t clocks)
{
    clockCounter += clocks;
    hCount = clockCounter / CLOCKS_PER_H;

    LogTimer("cc=%d h=%d v=%d", clockCounter, hCount, vCount);

    if (hCount > 1 && hCount < 274)
    {
        isHBlank = false;
        // Clear HBlank flag.
        Bytes::SetBit<6>(regHVBJOY);
    }
    else if (hCount >= 274 && hCount < 341)
    {
        isHBlank = true;
        // Set HBlank flag.
        Bytes::ClearBit<6>(regHVBJOY);
    }
    else if (clockCounter >= 1364)
    {
        clockCounter -= 1364;
        hCount = clockCounter / CLOCKS_PER_H;

        // TODO: Check for number of scanlines per screen in regSETINI.

        vCount++;
        if (vCount == 225)
        {
            isVBlank = true;
            // Set VBlank flags.
            Bytes::SetBit<7>(regRDNMI);
            Bytes::SetBit<7>(regHVBJOY);

            // If joypad auto read is enabled, toggle the busy flag.
            if (Bytes::GetBit<0>(regNMITIMEN))
                Bytes::SetBit<0>(regHVBJOY);
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
            isVBlank = false;
            // Clear VBlank flags.
            Bytes::ClearBit<7>(regRDNMI);
            Bytes::ClearBit<7>(regHVBJOY);
        }
    }

    NotifyObservers(clocks);
}


uint8_t Timer::ReadRegister(EIORegisters ioReg) const
{
    LogPpu("Timer::ReadRegister %04X", ioReg);

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
            return regTIMEUP;
        case eRegHVBJOY: // 0x4212
            return regHVBJOY;
        default:
            throw std::range_error(fmt("Timer doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Timer::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogPpu("Timer::WriteRegister %04X", ioReg);

    switch (ioReg)
    {
        case eRegNMITIMEN: // 0x4200
            regNMITIMEN = byte;
            LogTimer("NMITIMEN=%02X", byte);
            return true;
        case eRegHTIMEL: // 0x4207
            regHTIMEL = byte;
            LogTimer("HTIMEL=%02X NYI", byte);
            return true;
        case eRegHTIMEH: // 0x4208
            regHTIMEH = byte;
            LogTimer("HTIMEH=%02X NYI", byte);
            return true;
        case eRegVTIMEL: // 0x4209
            regVTIMEL = byte;
            LogTimer("VTIMEL=%02X NYI", byte);
            return true;
        case eRegVTIMEH: // 0x420A
            regVTIMEH = byte;
            LogTimer("VTIMEH=%02X NYI", byte);
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