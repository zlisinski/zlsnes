#ifndef ZLSNES_CORE_DMA_H
#define ZLSNES_CORE_DMA_H

#include "IoRegisterProxy.h"

class Memory;

class Dma : IoRegisterProxy
{
public:
    Dma(Memory *memory);
    virtual ~Dma() {}

private:
    uint8_t ReadRegister(EIORegisters ioReg) const override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    void RunDma();
    void RunHDma();

    Memory *memory;

    uint8_t &regMDMAEN;  // 0x420B Select General Purpose DMA Channel(s) and Start Transfer
    uint8_t &regHDMAEN;  // 0x420C Select H-Blank DMA (H-DMA) Channel(s)
    
    uint8_t *regDMAPx[8]; // 0x43n0 DMA/HDMA Parameters
    uint8_t *regBBADx[8]; // 0x43n1 DMA/HDMA I/O-Bus Address (PPU-Bus aka B-Bus)
    uint8_t *regA1TxL[8]; // 0x43n2 HDMA Table Start Address (low)  / DMA Curr Addr (low)
    uint8_t *regA1TxH[8]; // 0x43n3 HDMA Table Start Address (high) / DMA Curr Addr (high)
    uint8_t *regA1Bx[8];  // 0x43n4 HDMA Table Start Address (bank) / DMA Curr Addr (bank)
    uint8_t *regDASxL[8]; // 0x43n5 Indirect HDMA Address (low)  / DMA Byte-Counter (low)
    uint8_t *regDASxH[8]; // 0x43n6 Indirect HDMA Address (high) / DMA Byte-Counter (high)
    uint8_t *regDASBx[8]; // 0x43n7 Indirect HDMA Address (bank)
    uint8_t *regA2AxL[8]; // 0x43n8 HDMA Table Current Address (low)
    uint8_t *regA2AxH[8]; // 0x43n9 HDMA Table Current Address (high)
    uint8_t *regNTRLx[8]; // 0x43nA HDMA Line-Counter (from current Table entry)

    friend class DmaTest;
};

#endif