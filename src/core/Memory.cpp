#include "Memory.h"
#include "DebuggerInterface.h"
#include "Cartridge.h"
#include "Timer.h"


Memory::Memory(InfoInterface *infoInterface, DebuggerInterface *debuggerInterface) :
    wram(),
    ioPorts21(),
    ioPorts40(),
    ioPorts42(),
    ioPorts43(),
    expansion(),
    cart(NULL),
    debuggerInterface(debuggerInterface),
    infoInterface(infoInterface),
    timer(NULL)
{

}


Memory::~Memory()
{

}


void Memory::SetCartridge(Cartridge *cart)
{
    this->cart = cart;
}


void Memory::SetTimer(Timer *timer)
{
    this->timer = timer;
}


uint8_t Memory::Read8Bit(uint32_t addr)
{
    if ((addr & 0x408000) == 0) // Bank is in range 0x00-0x3F or 0x80-0xBF, and offset is in range 0x0000-0x7FFF.
    {
        if (HasIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF)))
            return ReadIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF));

        uint8_t page = Bytes::GetByte<1>(addr);
        switch (page)
        {
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
                timer->AddCycle(EClockSpeed::eClockWRam);
                return wram[addr & 0x1FFF];
            case 0x21:
                throw NotYetImplementedException(fmt("Read from unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockIoReg);
                //return ioPorts21[addr & 0xFF];
            case 0x40:
                throw NotYetImplementedException(fmt("Read from unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockOther);
                //return ioPorts40[addr & 0xFF];
            case 0x42:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                switch (addr)
                {
                    case eRegRDNMI:
                        {
                            uint8_t value = ioPorts42[addr & 0xFF];
                            if (value & 0x80)
                            {
                                // VBlank NMI flag gets reset after reads.
                                ioPorts42[addr & 0xFF] &= 0x7F;
                                if (debuggerInterface != nullptr)
                                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                            }
                            return value;
                        }
                    default:
                        throw NotYetImplementedException(fmt("Read from unhandled address %06X", addr));
                }
                throw NotYetImplementedException(fmt("Read from unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockIoReg);
                //return ioPorts42[addr & 0xFF];
            case 0x43:
                throw NotYetImplementedException(fmt("Read from unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockIoReg);
                //return ioPorts43[addr & 0xFF];
            default:
                LogError("Read from unhandled address %06X", addr);
                throw NotYetImplementedException(fmt("Read from unhandled address %06X", addr));
                return 0;
        }
    }

    if ((addr & 0xFE0000) == 0x7E0000) // Bank is 0x7E or 0x7F.
    {
        timer->AddCycle(EClockSpeed::eClockWRam);
        return wram[addr & 0x1FFFF];
    }

    if (addr & 0x8000)
    {
        // TODO: Figure out if this is slow or fast
        timer->AddCycle(EClockSpeed::eClockFastRom);

        // Assume LoROM for now.
        // Remove the high bit of the offset and shift the bank right one so that LSBit of bank is MSBit of offset.
        uint32_t mappedAddr = (((addr & 0xFF0000) >> 1) | (addr & 0x7FFF));
        return cart->GetRom()[mappedAddr];
    }

    // TODO: Figure out if this is slow or fast
    timer->AddCycle(EClockSpeed::eClockFastRom);

    LogError("Read from HiROM area %06X", addr);
    throw NotYetImplementedException(fmt("Read from HiROM area %06X", addr));
    return 0;
}


void Memory::Write8Bit(uint32_t addr, uint8_t value)
{
    if ((addr & 0x408000) == 0) // Bank is in range 0x00-0x3F or 0x80-0xBF, and offset is in range 0x0000-0x7FFF.
    {
        // Let observers handle the update. If there are no observers for this address, continue with normal processing.
        if (WriteIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF), value))
        {
            if (debuggerInterface != NULL)
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
                timer->AddCycle(EClockSpeed::eClockWRam);
                wram[addr & 0x1FFF] = value;
                if (debuggerInterface != NULL)
                    debuggerInterface->MemoryChanged(Address(0x7E, addr & 0x1FFF), 1);
                return;
            case 0x21:
                throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockIoReg);
                //ioPorts21[addr & 0xFF] = value;
                if (debuggerInterface != NULL)
                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                return;
            case 0x40:
                throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockOther);
                //ioPorts40[addr & 0xFF] = value;
                if (debuggerInterface != NULL)
                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                return;
            case 0x42:
                timer->AddCycle(EClockSpeed::eClockIoReg);
                switch (addr)
                {
                    case eRegNMITIMEN:
                        ioPorts42[addr & 0xFF] = value;
                        break;
                    case eRegMDMAEN:
                        ioPorts42[addr & 0xFF] = value;
                        RunDma();
                        break;
                    default:
                        throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                        //ioPorts42[addr & 0xFF] = value;
                        break;
                }
                //throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                //timer->AddCycle(EClockSpeed::eClockIoReg);
                //ioPorts42[addr & 0xFF] = value;
                if (debuggerInterface != NULL)
                    debuggerInterface->MemoryChanged(Address(addr & 0xFFFF), 1);
                return;
            case 0x43:
                //throw NotYetImplementedException(fmt("Write to unhandled address %06X", addr));
                timer->AddCycle(EClockSpeed::eClockIoReg);
                ioPorts43[addr & 0xFF] = value;
                if (debuggerInterface != NULL)
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
        timer->AddCycle(EClockSpeed::eClockWRam);
        wram[addr & 0x1FFFF] = value;
        if (debuggerInterface != NULL)
            debuggerInterface->MemoryChanged(Address(addr & 0x1FFFF), 1);
        return;
    }

    if (addr & 0x8000)
    {
        // TODO: Figure out if this is slow or fast
        timer->AddCycle(EClockSpeed::eClockFastRom);

        // Assume LoROM for now.
        // Remove the high bit of the offset and shift the bank right one so that LSBit of bank is MSBit of offset.
        uint32_t mappedAddr = (((addr & 0xFF0000) >> 1) | (addr & 0x7FFF));
        LogError("Attempting to write to ROM at %06X", mappedAddr);
        throw NotYetImplementedException("Attempting to write to ROM");
        return;
    }

    // TODO: Figure out if this is slow or fast
    timer->AddCycle(EClockSpeed::eClockFastRom);

    LogError("Write to HiROM area %06X", addr);
    throw NotYetImplementedException("Write to HiROM");
    return;
}


uint8_t *Memory::GetBytePtr(uint32_t addr)
{
    // LoROM
    if ((addr & 0x408000) == 0x8000)
        // TODO: fix this cast.
        return const_cast<uint8_t *>(&cart->GetRom()[((addr & 0xFF0000) >> 1) | (addr & 0x7FFF)]);

    // WRAM mirror.
    if ((addr & 0x40E000) == 0)
        return &wram[addr & 0x1FFFF];

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

    throw std::range_error(fmt("GetBytePtr(): Invalid address %06X", addr));
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


uint8_t *Memory::GetIoRegisterPtr(EIORegisters ioReg)
{
    switch (ioReg & 0xFF00)
    {
        case 0x2100:
            return &ioPorts21[ioReg & 0xFF];
        case 0x4000:
            return &ioPorts40[ioReg & 0xFF];
        case 0x4200:
            return &ioPorts42[ioReg & 0xFF];
        case 0x4300:
            return &ioPorts43[ioReg & 0xFF];
        default:
            throw std::range_error(fmt("Invalid IO register %04X", ioReg));
    }
}


void Memory::RunDma()
{
    uint8_t dmaEnable = ioPorts42[eRegMDMAEN & 0xFF];
    uint8_t hdmaEnable = ioPorts42[eRegHDMAEN & 0xFF];

    for (uint8_t channel = 0; channel < 8; channel++, dmaEnable >>= 1, hdmaEnable >>= 1)
    {
        // Check if this channel is enabled for GPDMA and not enabled for HDMA.
        if (!(dmaEnable & 0x01) || (hdmaEnable & 0x01))
            continue;

        uint8_t ch = channel << 4;
        uint8_t dmap = ioPorts43[ch | eDma_DMAPx];

        // Which direction is this transfer.
        bool bToA = Bytes::GetBit<7>(dmap);

        // The addresses on the A-bus and B-bus.
        uint32_t aBusAddr = Bytes::Make24Bit(ioPorts43[ch | eDma_A1Bx], ioPorts43[ch | eDma_A1TxH], ioPorts43[ch | eDma_A1TxL]);
        uint16_t bBusAddr = Bytes::Make16Bit(0x21, ioPorts43[ch | eDma_BBADx]);

        // How to increment the aBusAddr after each byte.
        int aBusInc = 0;
        // Odd numbers are fixed, 0 means +1, 2 means -1.
        if (Bytes::GetBit<3>(dmap) != 1)
            aBusInc = 1 - ((dmap >> 3) & 0x02);
        
        // How to increment the bBusAddr after each byte.
        // mode and the lower two bits of bbi are used to index into the bBusInc table.
        uint32_t mode = dmap & 0x07;
        uint32_t bbi = 0;
        uint8_t bBusInc[8][4] = {
            {0, 0, 0, 0}, // mode = 0
            {0, 1, 0, 1}, // mode = 1
            {0, 0, 0, 0}, // mode = 2
            {0, 0, 1, 1}, // mode = 3
            {0, 1, 2, 3}, // mode = 4
            {0, 1, 0, 1}, // mode = 5
            {0, 0, 0, 0}, // mode = 6
            {0, 0, 1, 1}  // mode = 7
        };
        
        // How many bytes to transfer. byteCount == 0 means 65536, since it will underflow.
        uint16_t byteCount = Bytes::Make16Bit(ioPorts43[ch | eDma_DASxH], ioPorts43[ch | eDma_DASxL]);

        LogInstruction("DMA%d: dmap=%02X, bToA=%d, aBusAddr=%06X, aBusInc=%d, bBusAddr=%04X, count=%d",
                       channel, dmap, bToA, aBusAddr, aBusInc, bBusAddr, (int)byteCount);

        do
        {
            // TODO: Add checking of invalid transfers (io ports using A-bus address to B-bus, WRAM to WRAM-through-io-port).
            if (bToA)
            {
                // TODO: Fix this. Reads from B-bus should go through IoRegisterProxy.
                //Write8Bit(aBusAddr, *GetBytePtr(bBusAddr + bBusInc[mode][bbi]));
                throw NotYetImplementedException("DMA from B-Bus to A-Bus NYI");
            }
            else
            {
                Write8Bit(bBusAddr + bBusInc[mode][bbi], *GetBytePtr(aBusAddr));
            }

            bbi = (bbi + 1) & 3;
            aBusAddr += aBusInc;
            byteCount--;
        }
        while (byteCount > 0);

        // Clear the enable bit for the channel when the transfer is done.
        ioPorts42[eRegMDMAEN & 0xFF] &= ~(1 << channel);
    }
}