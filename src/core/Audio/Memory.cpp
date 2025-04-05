#include "../IoRegisters.h"
#include "Memory.h"
#include "Timer.h"


namespace Audio
{


static const std::array<uint8_t, 64> bootRom = {
    0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0,
    0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
    0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4,
    0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
    0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB,
    0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
    0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD,
    0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};


Memory::Memory()
{
    ram[eRegTEST] = 0x0A;
    ram[eRegCONTROL] = 0x80;
    ram[eRegDSPADDR] = 0xFF;
    ram[eRegAUXIO4] = 0xFF;
    ram[eRegAUXIO5] = 0xFF;
    ram[eRegT0DIV] = 0xFF;
    ram[eRegT1DIV] = 0xFF;
    ram[eRegT2DIV] = 0xFF;
}


Memory::~Memory()
{

}


void Memory::WriteIoPort(uint8_t port, uint8_t byte)
{
    cpuReadPorts[port] = byte;
}


uint8_t Memory::Read8Bit(uint16_t addr)
{
    timer->AddCycle();

    switch (addr)
    {
        case eRegDSPDATA: // 0xF3
            LogSpcMem("Read from DSPDATA %02X NYI", ram[eRegDSPADDR]);
            return ram[eRegDSPDATA];
        case eRegCPUIO0: // 0xF4
            return cpuReadPorts[0];
        case eRegCPUIO1: // 0xF5
            return cpuReadPorts[1];
        case eRegCPUIO2: // 0xF6
            return cpuReadPorts[2];
        case eRegCPUIO3: // 0xF7
            return cpuReadPorts[3];
    }

    if (HasIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF)))
    {
        return ReadIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF));
    }

    if ((addr & 0xFFC0) == 0xFFC0 && bootRomEnabled)
    {
        return bootRom[addr & 0x3F];
    }

    return ram[addr];
}


void Memory::Write8Bit(uint16_t addr, uint8_t value)
{
    LogSpcMem("Memory::Write8Bit %04X, %02X", addr, value);

    timer->AddCycle();

    // Let observers handle the update. If there are no observers for this address, continue with normal processing.
    if (WriteIoRegisterProxy(static_cast<EIORegisters>(addr & 0xFFFF), value))
    {
        return;
    }

    switch (addr)
    {
        case eRegCONTROL: // 0xF1
            timer->EnableTimer0(Bytes::TestBit<0>(value));
            timer->EnableTimer1(Bytes::TestBit<1>(value));
            timer->EnableTimer2(Bytes::TestBit<2>(value));
            if (Bytes::TestBit<4>(value))
            {
                cpuReadPorts[0] = 0;
                cpuReadPorts[1] = 0;
                LogSpcMem("Clearing CPUIO 0-1");
            }
            if (Bytes::TestBit<5>(value))
            {
                cpuReadPorts[2] = 0;
                cpuReadPorts[3] = 0;
                LogSpcMem("Clearing CPUIO 2-3");
            }
            bootRomEnabled = Bytes::TestBit<7>(value);
            LogSpcMem("CONTROL=%02X t0=%d t1=%d t2=%d r01=%d r23=%d bootrom=%d", value,
                      Bytes::TestBit<0>(value), Bytes::TestBit<1>(value), Bytes::TestBit<2>(value),
                      Bytes::TestBit<4>(value), Bytes::TestBit<5>(value), bootRomEnabled);
            return;

        case eRegDSPDATA: // 0xF3
            LogSpcMem("Write to DSPDATA %02X=%02X NYI", ram[eRegDSPADDR], value);
            ram[eRegDSPDATA] = value;
            return;
    }

    ram[addr] = value;
}


void Memory::ClearMemory()
{
    ram.fill(0);
}


uint8_t *Memory::GetBytePtr(uint32_t addr)
{
    if (addr > 0xFFFF)
        throw std::range_error(fmt("Invalid address %08X", addr));

    return &ram[addr];
}


uint8_t &Memory::GetIoRegisterRef(EIORegisters ioReg)
{
    if (ioReg < 0xF0 || ioReg > 0xFF)
        throw std::range_error(fmt("Invalid IO register %04X", ioReg));

    return ram[ioReg];
}



}