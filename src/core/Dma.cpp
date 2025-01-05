#include "Dma.h"
#include "Memory.h"
#include "Timer.h"


static const uint8_t BBUS_INC_LOOKUP[8][4] = {
    {0, 0, 0, 0}, // mode = 0
    {0, 1, 0, 1}, // mode = 1
    {0, 0, 0, 0}, // mode = 2
    {0, 0, 1, 1}, // mode = 3
    {0, 1, 2, 3}, // mode = 4
    {0, 1, 0, 1}, // mode = 5
    {0, 0, 0, 0}, // mode = 6
    {0, 0, 1, 1}  // mode = 7
};

static const uint8_t HDMA_BYTES_LOOKUP[8] = {1, 2, 2, 4, 4, 4, 2, 4};


Dma::DmaChannelData::DmaChannelData(uint8_t channelId, uint8_t *regData) :
    channelId(channelId),
    regData(regData),
    doTransfer(false),
    isTerminated(true)
{

}


void Dma::DmaChannelData::SyncToMemory()
{
    // Write values that may have changed during HDMA back to memory so the debugger can see them.
    uint8_t ch = channelId << 4;
    regData[ch | eDma_A1TxH] = aBusOffset >> 8;
    regData[ch | eDma_A1TxL] = aBusOffset & 0xFF;
    regData[ch | eDma_DASxH] = byteCount >> 8;
    regData[ch | eDma_DASxL] = byteCount & 0xFF;
    regData[ch | eDma_A2AxH] = directOffset >> 8;
    regData[ch | eDma_A2AxL] = directOffset & 0xFF;
    regData[ch | eDma_NTRLx] = lineCount;
}


///////////////////////////////////////////////////////////////////////////////


Dma::Dma(Memory *memory, Timer *timer) :
    memory(memory),
    timer(timer),
    ioPorts43(memory->RequestOwnershipBlock(0x4300, 0x100, this)),
    regMDMAEN(memory->RequestOwnership(eRegMDMAEN, this)),
    regHDMAEN(memory->RequestOwnership(eRegHDMAEN, this)),
    channels{{0, ioPorts43},
             {1, ioPorts43},
             {2, ioPorts43},
             {3, ioPorts43},
             {4, ioPorts43},
             {5, ioPorts43},
             {6, ioPorts43},
             {7, ioPorts43}}
{
    // Initialize registers.
    regMDMAEN = 0;
    regHDMAEN = 0;
    for (int i = 0; i < 256; i++)
        ioPorts43[i] = 0xFF;

    timer->AttachHBlankObserver(this);
    timer->AttachVBlankObserver(this);
}


uint8_t Dma::ReadRegister(EIORegisters ioReg) const
{
    LogDma("Dma::ReadRegister %04X", ioReg);

    if ((ioReg >> 8) == 0x43)
    {
        timer->AddCycle(eClockIoReg);
        return ioPorts43[ioReg & 0xFF];
    }

    switch (ioReg)
    {
        default:
            throw std::range_error(fmt("Dma doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Dma::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogDma("Dma::WriteRegister %04X %02X", ioReg, byte);

    if ((ioReg >> 8) == 0x43)
    {
        uint8_t channel = (ioReg >> 4) & 0x07;
        switch (ioReg & 0x0F)
        {
            case eDma_DMAPx: // 0x43n0
                channels[channel].dmaParameters = byte;
                break;
            case eDma_BBADx: // 0x43n1
                channels[channel].bBusPort = byte;
                break;
            case eDma_A1TxL: // 0x43n2
                channels[channel].aBusOffset = (channels[channel].aBusOffset & 0xFF00) | byte;
                break;
            case eDma_A1TxH: // 0x43n3
                channels[channel].aBusOffset = (byte << 8) | (channels[channel].aBusOffset & 0xFF);
                break;
            case eDma_A1Bx: // 0x43n4
                channels[channel].aBusBank = byte;
                break;
            case eDma_DASxL: // 0x43n5
                channels[channel].byteCount = (channels[channel].byteCount & 0xFF00) | byte;
                break;
            case eDma_DASxH: // 0x43n6
                channels[channel].byteCount = (byte << 8) | (channels[channel].byteCount & 0xFF);
                break;
            case eDma_DASBx: // 0x43n7
                channels[channel].indirectBank = byte;
                break;
            case eDma_A2AxL: // 0x43n8
                channels[channel].indirectOffset = (channels[channel].indirectOffset & 0xFF00) | byte;
                break;
            case eDma_A2AxH: // 0x43n9
                channels[channel].indirectOffset = (byte << 8) | (channels[channel].indirectOffset & 0xFF);
                break;
            case eDma_NTRLx: // 0x43nA
                channels[channel].lineCount = byte;
                break;
        }

        // Write it to memory so the debugger can see the last written value.
        ioPorts43[ioReg & 0xFF] = byte;
        timer->AddCycle(eClockIoReg);

        return true;
    }

    switch (ioReg)
    {
        case eRegMDMAEN: // 0x420B
            regMDMAEN = byte;
            LogDma("MDMAEN=%02X", byte);
            RunDma();
            break;
        case eRegHDMAEN: // 0x420C
            regHDMAEN = byte;
            LogDma("HDMAEN=%02X", byte);
            break;
        default:
            throw std::range_error(fmt("Dma doesnt handle writes to 0x%04X", ioReg));
    }

    timer->AddCycle(eClockIoReg);
    return true;
}


void Dma::ProcessHBlankStart(uint32_t scanline)
{
    // Technically this is supposed to happen at h==278, not when hblank starts at h==274.
    if (regHDMAEN != 0 && scanline <= 224)
        RunHDma();
}


void Dma::ProcessVBlankEnd()
{
    // Technically this is supposed to happen at h==6 v==0, not when vblank ends on h==0 v==0.
    SetupHdma();
}


void Dma::RunDma()
{
    uint8_t dmaEnable = regMDMAEN;
    uint8_t hdmaEnable = regHDMAEN;

    for (uint8_t i = 0; i < 8; i++, dmaEnable >>= 1, hdmaEnable >>= 1)
    {
        // Check if this channel is enabled for GPDMA and not enabled for HDMA.
        if (!(dmaEnable & 0x01) || (hdmaEnable & 0x01))
            continue;

        LogDma("RunDma(): channel=%d", i);
        DmaChannelData &channel = channels[i];

        // How to increment the aBusAddr after each byte.
        int aBusInc = 0;
        if (!channel.dmap.fixed)
            aBusInc = 1 - (channel.dmap.decrementABus << 1);
        
        // How to increment the bBusAddr after each byte.
        // dmap.mode and the lower two bits of bbi are used to index into the BBUS_INC_LOOKUP table.
        uint32_t bbi = 0;

        LogDma("DMA%d: dmap=%02X, bToA=%d, aBusAddr=%02X%04X, aBusInc=%d, bBusAddr=21%02X, count=%04X",
                  i, channel.dmap, channel.dmap.bToA, channel.aBusBank, channel.aBusOffset, aBusInc, channel.bBusPort, channel.byteCount);

        // channel.byteCount == 0 means 65536, since it will underflow.
        do
        {
            // TODO: Add checking of invalid transfers (io ports using A-bus address to B-bus, WRAM to WRAM-through-io-port).
            if (channel.dmap.bToA)
            {
                // TODO: Fix this. Reads from B-bus should go through IoRegisterProxy.
                //Write8Bit(aBusAddr, *GetBytePtr(bBusAddr + bBusInc[mode][bbi]));
                throw NotYetImplementedException("DMA from B-Bus to A-Bus NYI");
            }
            else
            {
                Address aAddr(channel.aBusBank, channel.aBusOffset);
                Address bAddr(0, 0x21, channel.bBusPort + BBUS_INC_LOOKUP[channel.dmap.mode][bbi]);
                memory->Write8Bit(bAddr, *memory->GetBytePtr(aAddr.ToUint()));
                //LogDma("Dma %02X from %06X to %04X", *memory->GetBytePtr(aAddr.ToUint()), aAddr.ToUint(), bAddr.ToUint());
            }

            bbi = (bbi + 1) & 3;
            channel.aBusOffset += aBusInc;
            channel.byteCount--;
            channel.SyncToMemory();
        }
        while (channel.byteCount > 0);

        // Clear the enable bit for the channel when the transfer is done.
        regMDMAEN &= ~(1 << i);
    }
}


void Dma::SetupHdma()
{
    for (int i = 0; i < 8; i++)
    {
        DmaChannelData &channel = channels[i];
        channel.doTransfer = false;

        if ((regHDMAEN & (1 << i)) == 0)
            continue;

        LogDma("SetupHdma() channel=%d", i);

        channel.isTerminated = false;
        channel.directOffset = channel.aBusOffset;
        channel.lineCount = 0;

        LoadNextHdmaData(channel);
    }
}


void Dma::LoadNextHdmaData(DmaChannelData &channel)
{
    LogDma("LoadNextHdmaData() channel=%i", channel.channelId);

    uint8_t newLineCount = memory->Read8Bit(Address(channel.aBusBank, channel.directOffset));

    if ((channel.lineCount & 0x7F) == 0)
    {
        channel.lineCount = newLineCount;
        LogDma("  lineCount=%02X", channel.lineCount);
        channel.directOffset++;

        channel.doTransfer = !!channel.lineCount;
        channel.isTerminated = !channel.lineCount;

        if (channel.dmap.indirect)
        {
            channel.indirectOffset = memory->Read16Bit(Address(channel.aBusBank, channel.directOffset));
            channel.directOffset += 2;
            LogDma("  indirectAddress=%02X%04X", channel.indirectBank, channel.indirectOffset);
        }
    }

    channel.SyncToMemory();
}


void Dma::RunHDma()
{
    uint8_t hdmaEnable = regHDMAEN;

    for (uint8_t i = 0; i < 8; i++, hdmaEnable >>= 1)
    {
        DmaChannelData &channel = channels[i];

        // Check if this channel is enabled for HDMA.
        if (!(hdmaEnable & 0x01) || channel.isTerminated)
            continue;

        LogDma("  RunHDma() channel=%i term=%d", i, channel.isTerminated);

        if (channel.doTransfer)
        {
            uint8_t aBank = channel.dmap.indirect ? channel.indirectBank : channel.aBusBank;
            uint16_t &aOffset = channel.dmap.indirect ? channel.indirectOffset : channel.directOffset;

            LogDma("  HDMA%d: dmap=%02X, mode=%d, bToA=%d, aBusAddr=%02%04X, bBusAddr=21%02X, linecount=%02X",
                   i, channel.dmap, channel.dmap.mode, channel.dmap.bToA, aBank, aOffset, channel.bBusPort, channel.lineCount);

            for (int j = 0; j < HDMA_BYTES_LOOKUP[channel.dmap.mode]; j++)
            {
                Address aAddr(aBank, aOffset);
                Address bAddr(0, 0x21, channel.bBusPort + BBUS_INC_LOOKUP[channel.dmap.mode][j]);

                // Copy a byte.
                if (channel.dmap.bToA)
                {
                    uint8_t byte = memory->Read8Bit(bAddr);
                    LogDma("  HDMA %02X from %06X to %06X", byte, bAddr.ToUint(), aAddr.ToUint());
                    memory->Write8Bit(aAddr, byte);
                }
                else
                {
                    uint8_t byte = memory->Read8Bit(aAddr);
                    LogDma("  HDMA %02X from %06X to %06X", byte, aAddr.ToUint(), bAddr.ToUint());
                    memory->Write8Bit(bAddr, byte);
                }

                // Increment the A bus address.
                aOffset++;
            }
        }

        channel.lineCount--;
        channel.doTransfer = Bytes::TestBit<7>(channel.lineCount);
        if ((channel.lineCount & 0x7F) == 0)
            LoadNextHdmaData(channel);
        else
            channel.SyncToMemory();
    }
}