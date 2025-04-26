#include "Memory.h"
#include "DebuggerInterface.h"
#include "Cartridge.h"
#include "Ppu.h"
#include "Timer.h"


// The linker needs this, otherwise the function definition needs to go in the header.
template uint8_t Memory::Read8Bit<true>(uint32_t addr);
template uint8_t Memory::Read8Bit<false>(uint32_t addr);
template void Memory::Write8Bit<true>(uint32_t addr, uint8_t value);
template void Memory::Write8Bit<false>(uint32_t addr, uint8_t value);


Memory::Memory(InfoInterface *infoInterface, DebuggerInterface *debuggerInterface) :
    wram(),
    ioPorts21(),
    ioPorts40(),
    ioPorts42(),
    ioPorts43(),
    expansion(),
    wramRWAddr(0),
    isFastSpeed(false),
    openBusValue(0),
    cart(nullptr),
    debuggerInterface(debuggerInterface),
    infoInterface(infoInterface),
    ppu(nullptr),
    timer(nullptr)
{

}


Memory::~Memory()
{

}


template<bool addTime>
uint8_t Memory::Read8Bit(uint32_t addr)
{
    // Note: Only store the openBusValue on reads from ROM and RAM (assuming that code can be executed from RAM).
    // IO registers should never be part of the instruction.

    if ((addr & 0x40E000) < 0x6000) // Bank is in range 0x00-0x3F or 0x80-0xBF, and offset is in range 0x0000-0x5FFF.
    {
        if (HasIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF)))
        {
            // All io registers except for 4016/4017 take the same time.
            // The Input class will take care of adding additional cycles in those cases.
            if constexpr (addTime)
                timer->AddCycle(EClockSpeed::eClockIoReg);
            return ReadIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF));
        }

        uint8_t page = Bytes::GetByte<1>(addr);
        switch (page)
        {
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockWRam);
                // Save value for later open bus reads.
                openBusValue = wram[addr & 0x1FFF];
                return openBusValue;

            case 0x21:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                switch (addr & 0xFFFF)
                {
                    case eRegWMDATA: // 0x2180
                        {
                            uint8_t value = wram[wramRWAddr];
                            LogMemory("Read from wram through WMData %06X=%02X", wramRWAddr, value);
                            wramRWAddr = (wramRWAddr + 1) & 0x1FFFF;
                            return value;
                        }
                        break;
                }
                LogWarning("Read from unused IO port %06X", addr);
                return openBusValue;

            case 0x40:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockOther);
                LogWarning("Read from unused IO port %06X", addr);
                return openBusValue;

            case 0x41:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                LogWarning("Read from unused IO port %06X", addr);
                return openBusValue;

            case 0x42:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                switch (addr & 0xFFFF)
                {
                    case eRegRDDIVH:
                    case eRegRDDIVL:
                    case eRegRDMPYH:
                    case eRegRDMPYL:
                        return ioPorts42[addr & 0xFF];
                    default:
                        return openBusValue;
                }

            default:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                LogWarning("Read from unhandled address %06X", addr);
                return openBusValue;
        }
    }

    if ((addr & 0xFE0000) == 0x7E0000) // Bank is 0x7E or 0x7F.
    {
        if constexpr (addTime)
            timer->AddCycle(EClockSpeed::eClockWRam);
        // Save value for later open bus reads.
        openBusValue = wram[addr & 0x1FFFF];
        return openBusValue;
    }

    if constexpr (addTime)
    {
        if ((addr & 0x800000) == 0x800000 && isFastSpeed)
            timer->AddCycle(EClockSpeed::eClockFastRom);
        else
            timer->AddCycle(EClockSpeed::eClockSlowRom);
    }

    openBusValue = cart->ReadByte(addr);
    return openBusValue;
}


template<bool addTime>
void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    if ((addr & 0x40E000) < 0x6000) // Bank is in range 0x00-0x3F or 0x80-0xBF, and offset is in range 0x0000-0x5FFF.
    {
        // Let observers handle the update. If there are no observers for this address, continue with normal processing.
        if (WriteIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF), value))
        {
            // All io registers except for 4016/4017 take the same time.
            // The Input class will take care of adding additional cycles in those cases.
            if constexpr (addTime)
                timer->AddCycle(EClockSpeed::eClockIoReg);

            if (debuggerInterface && debuggerInterface->GetDebuggingEnabled())
                debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
            return;
        }

        uint8_t page = Bytes::GetByte<1>(addr);
        switch (page)
        {
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockWRam);
                wram[addr & 0x1FFF] = value;
                if (debuggerInterface && debuggerInterface->GetDebuggingEnabled())
                    debuggerInterface->MemoryChanged(Address(0x7E, addr & 0x1FFF), 1);
                return;
            case 0x21:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                switch (addr & 0xFFFF)
                {
                    case eRegWMDATA: // 0x2180
                        wram[wramRWAddr] = value;
                        LogMemory("Write to wram through WMData %06X=%02X", wramRWAddr, value);
                        wramRWAddr = (wramRWAddr + 1) & 0x1FFFF; // This may be meant to update the WMADD[HML] registers.
                        break;
                    case eRegWMADDL: // 0x2181
                        ioPorts21[addr & 0xFF] = value;
                        wramRWAddr = Bytes::Make24Bit(ioPorts21[eRegWMADDH & 0xFF] & 0x01, ioPorts21[eRegWMADDM & 0xFF], value);
                        LogMemory("wramRwAddr=%06X", wramRWAddr);
                        break;
                    case eRegWMADDM: // 0x2182
                        ioPorts21[addr & 0xFF] = value;
                        wramRWAddr = Bytes::Make24Bit(ioPorts21[eRegWMADDH & 0xFF] & 0x01, value, ioPorts21[eRegWMADDL & 0xFF]);
                        LogMemory("wramRwAddr=%06X", wramRWAddr);
                        break;
                    case eRegWMADDH: // 0x2183
                        ioPorts21[addr & 0xFF] = value;
                        wramRWAddr = Bytes::Make24Bit(value & 0x01, ioPorts21[eRegWMADDM & 0xFF], ioPorts21[eRegWMADDL & 0xFF]);
                        LogMemory("wramRwAddr=%06X", wramRWAddr);
                        break;
                    default:
                        LogWarning("Write to unhandled address %06X", addr);
                        break;
                }
                if (debuggerInterface && debuggerInterface->GetDebuggingEnabled())
                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                return;
            case 0x40:
                //throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockOther);
                ioPorts40[addr & 0xFF] = value;
                LogMemory("Write to joypad port %04X %02X", addr & 0xFFFF, value);
                if (debuggerInterface && debuggerInterface->GetDebuggingEnabled())
                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                return;
            case 0x41:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                LogError("Write to unused IO port %06X %02X", addr, value);
                return;
            case 0x42:
                if constexpr (addTime)
                    timer->AddCycle(EClockSpeed::eClockIoReg);
                switch (addr & 0xFFFF)
                {
                    case eRegWRIO: // 0x4201
                        // On 1 to 0 transition, have the ppu latch counters.
                        if (Bytes::TestBit<7>(ioPorts42[addr & 0xFF]) && !Bytes::TestBit<7>(value))
                            ppu->LatchCounters();
                        ioPorts42[addr & 0xFF] = value;
                        LogMemory("WRIO=%02X NYI", value);
                        break;
                    case eRegWRMPYA: // 0x4202
                        ioPorts42[addr & 0xFF] = value;
                        break;
                    case eRegWRMPYB: // 0x4203
                    {
                        ioPorts42[addr & 0xFF] = value;
                        uint16_t result = value * ioPorts42[eRegWRMPYA & 0xFF];
                        ioPorts42[eRegRDMPYH & 0xFF] = Bytes::GetByte<1>(result);
                        ioPorts42[eRegRDMPYL & 0xFF] = Bytes::GetByte<0>(result);
                        break;
                    }
                    case eRegWRDIVL: // 0x4204
                        ioPorts42[addr & 0xFF] = value;
                        break;
                    case eRegWRDIVH: // 0x4205
                        ioPorts42[addr & 0xFF] = value;
                        break;
                    case eRegWRDIVB: // 0x4206
                    {
                        ioPorts42[addr & 0xFF] = value;
                        if (value != 0)
                        {
                            uint16_t dividend = Bytes::Make16Bit(ioPorts42[eRegWRDIVH & 0xFF], ioPorts42[eRegWRDIVL & 0xFF]);
                            uint16_t result = dividend / value;
                            uint16_t remain = dividend % value;
                            ioPorts42[eRegRDDIVH & 0xFF] = Bytes::GetByte<1>(result);
                            ioPorts42[eRegRDDIVL & 0xFF] = Bytes::GetByte<0>(result);
                            ioPorts42[eRegRDMPYH & 0xFF] = Bytes::GetByte<1>(remain);
                            ioPorts42[eRegRDMPYL & 0xFF] = Bytes::GetByte<0>(remain);
                        }
                        else
                        {
                            ioPorts42[eRegRDDIVH & 0xFF] = 0xFF;
                            ioPorts42[eRegRDDIVL & 0xFF] = 0xFF;
                            ioPorts42[eRegRDMPYH & 0xFF] = ioPorts42[eRegWRDIVH & 0xFF];
                            ioPorts42[eRegRDMPYL & 0xFF] = ioPorts42[eRegWRDIVL & 0xFF];
                        }
                        break;
                    }
                    case eRegMEMSEL: // 0x420D
                        ioPorts42[addr & 0xFF] = value;
                        if ((value & 0x01) /*&& cart->IsFastSpeed()*/) // Other emulators don't check the cart speed, but it seems wrong not to.
                            isFastSpeed = true;
                        else
                            isFastSpeed = false;
                        LogMemory("MEMSEL: %s %s", (value & 0x01) ? "fast" : "slow", isFastSpeed ? "fast" : "slow");
                        break;
                    default:
                        LogError("Write to read-only IO port %06X %02X", addr, value);
                        break;
                }
                //throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                //if constexpr (addTime)
                //    timer->AddCycle(EClockSpeed::eClockIoReg);
                //ioPorts42[addr & 0xFF] = value;
                if (debuggerInterface && debuggerInterface->GetDebuggingEnabled())
                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                return;
            default:
                LogError("Write to unhandled address %06X", addr);
                throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                return;
        }
    }

    if ((addr & 0xFE0000) == 0x7E0000) // Bank is 0x7E or 0x7F.
    {
        if constexpr (addTime)
            timer->AddCycle(EClockSpeed::eClockWRam);
        wram[addr & 0x1FFFF] = value;
        if (debuggerInterface && debuggerInterface->GetDebuggingEnabled())
            debuggerInterface->MemoryChanged(Address(addr & 0x1FFFF), 1);
        return;
    }

    if constexpr (addTime)
    {
        if ((addr & 0x800000) == 0x800000 && isFastSpeed)
            timer->AddCycle(EClockSpeed::eClockFastRom);
        else
            timer->AddCycle(EClockSpeed::eClockSlowRom);
    }

    cart->WriteByte(addr, value);
}


uint8_t *Memory::GetBytePtr(uint32_t addr)
{
    // WRAM mirror.
    if ((addr & 0x40E000) == 0)
        return &wram[addr & 0x1FFF];

    // WRAM.
    if ((addr & 0xFE0000) == 0x7E0000)
        return &wram[addr & 0x1FFFF];
    
    // 0x2100 IO ports.
    if ((addr & 0x40FF00) == 0x2100)
        return &ioPorts21[addr & 0xFF];
    
    // 0x4000 IO ports.
    if ((addr & 0x40FF00) == 0x4000)
        return &ioPorts40[addr & 0xFF];
    
    // 0x4200 IO ports.
    if ((addr & 0x40FF00) == 0x4200)
        return &ioPorts42[addr & 0xFF];
    
    // 0x4300 IO ports.
    if ((addr & 0x40FF00) == 0x4300)
        return &ioPorts43[addr & 0xFF];

    return cart->GetBytePtr(addr);
}


void Memory::ClearMemory()
{
    wram.fill(0);
    ioPorts21.fill(0);
    ioPorts40.fill(0);
    ioPorts42.fill(0);
    ioPorts43.fill(0);
    expansion.fill(0);
}


uint8_t &Memory::GetIoRegisterRef(EIORegisters ioReg)
{
    switch (ioReg & 0xFF00)
    {
        case 0x2100:
            return ioPorts21[ioReg & 0xFF];
        case 0x4000:
            return ioPorts40[ioReg & 0xFF];
        case 0x4200:
            return ioPorts42[ioReg & 0xFF];
        case 0x4300:
            return ioPorts43[ioReg & 0xFF];
        default:
            throw std::range_error(fmt("Invalid IO register %04X", ioReg));
    }
}
