#include "Dma.h"
#include "Memory.h"


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
    doTransfer(false)
{
    Load();
}


void Dma::DmaChannelData::Load()
{
    uint8_t ch = (channelId << 4);
    dmaParameters = regData[ch | eDma_DMAPx];
    bBusAddr = Address(0, 0x21, regData[ch | eDma_BBADx]);
    aBusAddr = Address(regData[ch | eDma_A1Bx], regData[ch | eDma_A1TxH], regData[ch | eDma_A1TxL]);
    byteCount = Bytes::Make16Bit(regData[ch | eDma_DASxH], regData[ch | eDma_DASxL]);
    indirectAddr = Address(regData[ch | eDma_DASBx], regData[ch | eDma_DASxH], regData[ch | eDma_DASxL]);
    directAddr = Address(regData[ch | eDma_A1Bx], regData[ch | eDma_A2AxH], regData[ch | eDma_A2AxL]);
    lineCount = regData[ch | eDma_NTRLx];
}


void Dma::DmaChannelData::SaveABusAddr()
{
    uint8_t ch = (channelId << 4);
    uint16_t offset = aBusAddr.GetOffset();
    regData[ch | eDma_A1TxH] = offset >> 8;
    regData[ch | eDma_A1TxL] = offset & 0xFF;
}


void Dma::DmaChannelData::SaveByteCount()
{
    uint8_t ch = (channelId << 4);
    regData[ch | eDma_DASxH] = byteCount >> 8;
    regData[ch | eDma_DASxL] = byteCount & 0xFF;
}


///////////////////////////////////////////////////////////////////////////////


Dma::Dma(Memory *memory, TimerSubject *timer) :
    memory(memory),
    ioPorts43(memory->GetBytePtr(0x4300)),
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
}


uint8_t Dma::ReadRegister(EIORegisters ioReg) const
{
    LogDma("Dma::ReadRegister %04X", ioReg);

    switch (ioReg)
    {
        default:
            throw std::range_error(fmt("Dma doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Dma::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogDma("Dma::WriteRegister %04X %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegMDMAEN: // 0x420B
            regMDMAEN = byte;
            RunDma();
            return true;
        case eRegHDMAEN: // 0x420C
            regHDMAEN = byte;
            LogDma("HDMAEN=%02X NYI", byte);
            return true;
        default:
            throw std::range_error(fmt("Dma doesnt handle writes to 0x%04X", ioReg));
    }

    return false;
}


void Dma::ProcessHBlankStart(uint32_t scanline)
{
    // Technically this is supposed to happen at h==278, not when hblank starts at h==274.
    if (regHDMAEN != 0 && scanline <= 224)
        RunHDma(scanline);
}


void Dma::ProcessHBlankEnd(uint32_t scanline)
{
    if (scanline != 0)
        return;

    // Technically this is supposed to happen at h==6 v==0, not when hblank ends on h==1 v==0.
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

        DmaChannelData &channel = channels[i];
        channel.Load();

        // How to increment the aBusAddr after each byte.
        int aBusInc = 0;
        if (!channel.dmap.fixed)
            aBusInc = 1 - (channel.dmap.decrementABus << 1);
        
        // How to increment the bBusAddr after each byte.
        // dmap.mode and the lower two bits of bbi are used to index into the BBUS_INC_LOOKUP table.
        uint32_t bbi = 0;

        LogDma("DMA%d: dmap=%02X, bToA=%d, aBusAddr=%06X, aBusInc=%d, bBusAddr=%04X, count=%hu",
                  i, channel.dmap, channel.dmap.bToA, channel.aBusAddr.ToUint(), aBusInc, channel.bBusAddr.ToUint(), channel.byteCount);

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
                Address bAddr = channel.bBusAddr.AddOffsetWrapBank(BBUS_INC_LOOKUP[channel.dmap.mode][bbi]);
                memory->Write8Bit(bAddr, *memory->GetBytePtr(channel.aBusAddr.ToUint()));
            }

            bbi = (bbi + 1) & 3;
            channel.aBusAddr = channel.aBusAddr.AddOffsetWrapBank(aBusInc);
            channel.SaveABusAddr();
            channel.byteCount--;
            channel.SaveByteCount();
        }
        while (channel.byteCount > 0);

        // Clear the enable bit for the channel when the transfer is done.
        regMDMAEN &= ~(1 << i);
    }
}


void Dma::SetupHdma()
{

}


void Dma::RunHDma(uint32_t scanline)
{
    (void)scanline;
}