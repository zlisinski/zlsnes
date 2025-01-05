#ifndef ZLSNES_CORE_DMA_H
#define ZLSNES_CORE_DMA_H

#include "Address.h"
#include "Bytes.h"
#include "IoRegisterProxy.h"
#include "TimerObserver.h"


class Memory;
class Timer;


class Dma : public IoRegisterProxy, public HBlankObserver, public VBlankObserver
{
public:
    Dma(Memory *memory, Timer *timer);
    virtual ~Dma() {}

protected:
    // Inherited from HBlankObserver.
    void ProcessHBlankStart(uint32_t scanline) override;
    void ProcessHBlankEnd(uint32_t scanline) override {(void)scanline;}
    // Inherited from VBlankObserver.
    void ProcessVBlankStart() override {};
    void ProcessVBlankEnd() override;

private:
    // Inherited from IoRegisterProxy.
    uint8_t ReadRegister(EIORegisters ioReg) const override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    struct DmaChannelData
    {
        DmaChannelData(uint8_t channelId, uint8_t *regData);
        void SyncToMemory();

        uint8_t channelId;
        uint8_t *regData;
        bool doTransfer;
        bool isTerminated;

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

        uint8_t bBusPort; // 0x43n1 - BBADn
        uint8_t aBusBank; // 0x43n4 - A1TnB
        uint16_t aBusOffset; // 0x43n3,2 - A1TnH/A1Bn

        union // 0x43n6,5 - DASnH/DASnL
        {
            uint16_t byteCount; // DMA
            uint16_t indirectOffset; // HDMA
        };

        // HDMA
        uint8_t indirectBank; // 0x43n7 - DASBn
        uint16_t directOffset; // 0x43n9,8 - A2AnH/A2AnL
        uint8_t lineCount; // 0x43nA - NLTRn
    };

    void RunDma();
    void SetupHdma();
    void LoadNextHdmaData(DmaChannelData &channel);
    void RunHDma();

    Memory *memory;
    Timer *timer;
    uint8_t *ioPorts43; // This class basically owns this block of data, but I don't currently have a way of
                        // registering blocks of data, so this is the workaround for now.

    uint8_t &regMDMAEN;  // 0x420B Select General Purpose DMA Channel(s) and Start Transfer
    uint8_t &regHDMAEN;  // 0x420C Select H-Blank DMA (H-DMA) Channel(s)

    DmaChannelData channels[8];

    friend class DmaTest;
};

#endif