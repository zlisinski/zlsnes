#include "Dma.h"
#include "Memory.h"


Dma::Dma(Memory *memory) :
    memory(memory),
    regMDMAEN(memory->RequestOwnership(eRegMDMAEN, this)),
    regHDMAEN(memory->RequestOwnership(eRegHDMAEN, this))
{
    for (int i = 0; i < 8; i++)
    {
        regDMAPx[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_DMAPx), this);
        regBBADx[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_BBADx), this);
        regA1TxL[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_A1TxL), this);
        regA1TxH[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_A1TxH), this);
        regA1Bx[i]  = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_A1Bx), this);
        regDASxL[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_DASxL), this);
        regDASxH[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_DASxH), this);
        regDASBx[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_DASBx), this);
        regA2AxL[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_A2AxL), this);
        regA2AxH[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_A2AxH), this);
        regNTRLx[i] = &memory->RequestOwnership(static_cast<EIORegisters>(0x4300 | (i << 4) | eDma_NTRLx), this);
    }
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
    LogDma("Dma::WriteRegister %04X", ioReg);

    if ((ioReg >> 8) == 0x43)
    {
        uint8_t channel = (ioReg >> 4) & 0x07;
        switch (ioReg & 0x0F)
        {
            case eDma_DMAPx:
                *regDMAPx[channel] = byte;
                return true;
            case eDma_BBADx:
                *regBBADx[channel] = byte;
                return true;
            case eDma_A1TxL:
                *regA1TxL[channel] = byte;
                return true;
            case eDma_A1TxH:
                *regA1TxH[channel] = byte;
                return true;
            case eDma_A1Bx:
                *regA1Bx[channel] = byte;
                return true;
            case eDma_DASxL:
                *regDASxL[channel] = byte;
                return true;
            case eDma_DASxH:
                *regDASxH[channel] = byte;
                return true;
            case eDma_DASBx:
                *regDASBx[channel] = byte;
                return true;
            case eDma_A2AxL:
                *regA2AxL[channel] = byte;
                return true;
            case eDma_A2AxH:
                *regA2AxH[channel] = byte;
                return true;
            case eDma_NTRLx:
                *regNTRLx[channel] = byte;
                return true;
        }
    }

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


void Dma::RunDma()
{
    uint8_t dmaEnable = regMDMAEN;
    uint8_t hdmaEnable = regHDMAEN;

    for (uint8_t channel = 0; channel < 8; channel++, dmaEnable >>= 1, hdmaEnable >>= 1)
    {
        // Check if this channel is enabled for GPDMA and not enabled for HDMA.
        if (!(dmaEnable & 0x01) || (hdmaEnable & 0x01))
            continue;

        uint8_t dmap = *regDMAPx[channel];

        // Which direction is this transfer.
        bool bToA = Bytes::GetBit<7>(dmap);

        // The addresses on the A-bus and B-bus.
        uint32_t aBusAddr = Bytes::Make24Bit(*regA1Bx[channel], *regA1TxH[channel], *regA1TxL[channel]);
        uint16_t bBusAddr = Bytes::Make16Bit(0x21, *regBBADx[channel]);

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
        uint16_t byteCount = Bytes::Make16Bit(*regDASxH[channel], *regDASxL[channel]);

        LogDma("DMA%d: dmap=%02X, bToA=%d, aBusAddr=%06X, aBusInc=%d, bBusAddr=%04X, count=%d",
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
                memory->Write8Bit(bBusAddr + bBusInc[mode][bbi], *memory->GetBytePtr(aBusAddr));
            }

            bbi = (bbi + 1) & 3;
            aBusAddr += aBusInc;
            byteCount--;
        }
        while (byteCount > 0);

        // Clear the enable bit for the channel when the transfer is done.
        regMDMAEN &= ~(1 << channel);
    }
}


void Dma::RunHDma()
{

}