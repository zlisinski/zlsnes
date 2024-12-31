#ifndef ZLSNES_CORE_DMA_H
#define ZLSNES_CORE_DMA_H

#include "Address.h"
#include "Bytes.h"
#include "IoRegisterProxy.h"
#include "TimerObserver.h"

class Memory;


class Dma : public IoRegisterProxy, public HBlankObserver
{
public:
    Dma(Memory *memory, TimerSubject *timer);
    virtual ~Dma() {}

protected:
    // Inherited from HBlankObserver.
    void ProcessHBlankStart(uint32_t scanline) override;
    void ProcessHBlankEnd(uint32_t scanline) override;

private:
    // Inherited from IoRegisterProxy.
    uint8_t ReadRegister(EIORegisters ioReg) const override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    void RunDma();
    void SetupHdma();
    void RunHDma(uint32_t scanline);

    Memory *memory;
    uint8_t *ioPorts43; // This class basically owns this block of data, but I don't currently have a way of
                        // registering blocks of data, so this is the workaround for now.

    uint8_t &regMDMAEN;  // 0x420B Select General Purpose DMA Channel(s) and Start Transfer
    uint8_t &regHDMAEN;  // 0x420C Select H-Blank DMA (H-DMA) Channel(s)

    struct DmaChannelData
    {
        DmaChannelData(uint8_t channelId, uint8_t *regData);
        void Load();
        void SaveABusAddr();
        void SaveByteCount();

        uint8_t channelId;
        uint8_t *regData;
        bool doTransfer;

        union
        {
            uint8_t dmaParameters; // 0x43n0 - DMAPn
            struct
            {
                uint8_t mode:3;
                uint8_t fixed:1;
                uint8_t decrementABus:1;
                uint8_t unused:1;
                uint8_t indirect:1;
                uint8_t bToA:1;
            } dmap;
        };
        Address bBusAddr; // 0x43n1 - BBADn
        Address aBusAddr; // 0x43n4,3,2 - A1TnB/A1TnH/A1Bn

        // MPDMA
        uint16_t byteCount; // 0x43n6,5 - DASnL/DASnH

        // HDMA
        Address indirectAddr; // 0x43n7,6,5 - DASBn/DASnH/DASnL
        Address directAddr; // 0x43n4,9,8 - A1TnB/A2AnH/A2AnL
        uint8_t lineCount; // 0x43nA - NLTRn
    } channels[8];

    

    friend class DmaTest;
};

#endif