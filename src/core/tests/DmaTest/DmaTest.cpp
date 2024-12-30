#include <gtest/gtest.h>

// Include the mocks first so they override subsequent includes.
#include "mock/Memory.h"

#include "Dma.h"
#include "../TestLogger.h"

class DmaTest : public ::testing::Test
{
protected:
    DmaTest();
    ~DmaTest() override;

    void SetUp() override;
    void TearDown() override;

    // Used for testing private methods.
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) {return dma->WriteRegister(ioReg, byte);}

    Dma *dma;
    Memory *memory;
    Timer *timer;
    //TestLogger logger;
};


DmaTest::DmaTest()
{
    memory = new Memory();
    dma = new Dma(memory);
    
}

DmaTest::~DmaTest()
{
    delete dma;
    delete memory;
}

void DmaTest::SetUp()
{

}

void DmaTest::TearDown()
{

}

class FakeIo : public IoRegisterProxy
{
public:
    uint8_t ReadRegister(EIORegisters ioReg) const override
    {
        (void)ioReg;
        return 0;
    }

    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override
    {
        bytesWritten.emplace(ioReg, byte);
        return true;
    }

    std::unordered_multimap<EIORegisters, uint8_t> bytesWritten;
};


TEST_F(DmaTest, TEST_Dma1)
{
    FakeIo fakePpu;
    memory->RequestOwnership(eRegVMDATAL, &fakePpu);
    memory->RequestOwnership(eRegVMDATAH, &fakePpu);

    uint8_t *wram = memory->GetBytePtr(WRAM_OFFSET);
    wram[0x1234] = 0x11;
    wram[0x1235] = 0x22;
    wram[0x1236] = 0x11;
    wram[0x1237] = 0x22;
    wram[0x1238] = 0x11;
    wram[0x1239] = 0x22;
    wram[0x123A] = 0x11;
    wram[0x123B] = 0x22;
    wram[0x123C] = 0x11;
    wram[0x123D] = 0x22;
    wram[0x123E] = 0x11;
    wram[0x123F] = 0x22;
    wram[0x1240] = 0x11;
    wram[0x1241] = 0x22;
    wram[0x1242] = 0x11;
    wram[0x1243] = 0x22;

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x01); // AtoB, B increment mode 1, +1 A increment.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x34); // Read from 0x7E1234
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x10); // Write 16 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 16 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 8);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 8);

    auto range = fakePpu.bytesWritten.equal_range(eRegVMDATAL);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x11);
    }

    range = fakePpu.bytesWritten.equal_range(eRegVMDATAH);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x22);
    }
}