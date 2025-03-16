#include <gtest/gtest.h>

// Include the mocks first so they override subsequent includes.
#include "mock/Memory.h"
#include "../CommonMocks/Timer.h"

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
    timer = new Timer();
    dma = new Dma(memory, timer);
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
    virtual ~FakeIo() {}

    uint8_t ReadRegister(EIORegisters ioReg) override
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


TEST_F(DmaTest, TEST_Dma1_mode0)
{
    FakeIo fakePpu;
    memory->RequestOwnership(eRegVMDATAL, &fakePpu);
    memory->RequestOwnership(eRegVMDATAH, &fakePpu);

    uint8_t *wram = memory->GetBytePtr(WRAM_OFFSET);
    wram[0x1234] = 0x11;
    wram[0x1235] = 0x11;
    wram[0x1236] = 0x11;
    wram[0x1237] = 0x11;
    wram[0x1238] = 0x11;
    wram[0x1239] = 0x11;
    wram[0x123A] = 0x11;
    wram[0x123B] = 0x11;

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x00); // AtoB, B increment mode 0, +1 A increment.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x34); // Read from 0x7E1234
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x08); // Write 8 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 8 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 8);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 0);

    auto range = fakePpu.bytesWritten.equal_range(eRegVMDATAL);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x11);
    }

    // Byte count should be zero.
    EXPECT_EQ(memory->Read8Bit(0x4305), 0);
    EXPECT_EQ(memory->Read8Bit(0x4306), 0);

    // A bus increment.
    EXPECT_EQ(memory->Read8Bit(0x4302), 0x3C);
}


TEST_F(DmaTest, TEST_Dma1_mode1)
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

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x01); // AtoB, B increment mode 1, +1 A increment.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x34); // Read from 0x7E1234
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x08); // Write 16 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 16 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 4);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 4);

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

    // Byte count should be zero.
    EXPECT_EQ(memory->Read8Bit(0x4305), 0);
    EXPECT_EQ(memory->Read8Bit(0x4306), 0);

    // A bus increment.
    EXPECT_EQ(memory->Read8Bit(0x4302), 0x3C);
}


TEST_F(DmaTest, TEST_Dma1_mode3)
{
    FakeIo fakePpu;
    memory->RequestOwnership(eRegVMDATAL, &fakePpu);
    memory->RequestOwnership(eRegVMDATAH, &fakePpu);

    uint8_t *wram = memory->GetBytePtr(WRAM_OFFSET);
    wram[0x1234] = 0x11;
    wram[0x1235] = 0x11;
    wram[0x1236] = 0x22;
    wram[0x1237] = 0x22;
    wram[0x1238] = 0x11;
    wram[0x1239] = 0x11;
    wram[0x123A] = 0x22;
    wram[0x123B] = 0x22;

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x03); // AtoB, B increment mode 1, +1 A increment.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x34); // Read from 0x7E1234
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x08); // Write 16 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 16 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 4);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 4);

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

    // Byte count should be zero.
    EXPECT_EQ(memory->Read8Bit(0x4305), 0);
    EXPECT_EQ(memory->Read8Bit(0x4306), 0);

    // A bus increment.
    EXPECT_EQ(memory->Read8Bit(0x4302), 0x3C);
}


TEST_F(DmaTest, TEST_Dma1_mode4)
{
    FakeIo fakePpu;
    memory->RequestOwnership(eRegVMDATAL, &fakePpu);
    memory->RequestOwnership(eRegVMDATAH, &fakePpu);
    memory->RequestOwnership(eRegM7SEL, &fakePpu);
    memory->RequestOwnership(eRegM7A, &fakePpu);

    uint8_t *wram = memory->GetBytePtr(WRAM_OFFSET);
    wram[0x1234] = 0x11;
    wram[0x1235] = 0x22;
    wram[0x1236] = 0x33;
    wram[0x1237] = 0x44;
    wram[0x1238] = 0x11;
    wram[0x1239] = 0x22;
    wram[0x123A] = 0x33;
    wram[0x123B] = 0x44;

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x04); // AtoB, B increment mode 1, +1 A increment.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x34); // Read from 0x7E1234
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x08); // Write 16 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 16 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 2);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 2);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegM7SEL), 2);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegM7A), 2);

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

    range = fakePpu.bytesWritten.equal_range(eRegM7SEL);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x33);
    }

    range = fakePpu.bytesWritten.equal_range(eRegM7A);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x44);
    }

    // Byte count should be zero.
    EXPECT_EQ(memory->Read8Bit(0x4305), 0);
    EXPECT_EQ(memory->Read8Bit(0x4306), 0);

    // A bus increment.
    EXPECT_EQ(memory->Read8Bit(0x4302), 0x3C);
}


TEST_F(DmaTest, TEST_Dma1_FixedABus)
{
    FakeIo fakePpu;
    memory->RequestOwnership(eRegVMDATAL, &fakePpu);
    memory->RequestOwnership(eRegVMDATAH, &fakePpu);

    uint8_t *wram = memory->GetBytePtr(WRAM_OFFSET);
    wram[0x1234] = 0x11;
    wram[0x1235] = 0x22;
    wram[0x1236] = 0x33;
    wram[0x1237] = 0x44;
    wram[0x1238] = 0x55;
    wram[0x1239] = 0x66;
    wram[0x123A] = 0x77;
    wram[0x123B] = 0x88;

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x08); // AtoB, B increment mode 0, No A increment.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x34); // Read from 0x7E1234
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x08); // Write 8 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 8 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 8);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 0);

    auto range = fakePpu.bytesWritten.equal_range(eRegVMDATAL);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x11);
    }

    // Byte count should be zero.
    EXPECT_EQ(memory->Read8Bit(0x4305), 0);
    EXPECT_EQ(memory->Read8Bit(0x4306), 0);

    // A bus increment.
    EXPECT_EQ(memory->Read8Bit(0x4302), 0x34);
}


TEST_F(DmaTest, TEST_Dma1_DecrementABus)
{
    FakeIo fakePpu;
    memory->RequestOwnership(eRegVMDATAL, &fakePpu);
    memory->RequestOwnership(eRegVMDATAH, &fakePpu);

    uint8_t *wram = memory->GetBytePtr(WRAM_OFFSET);
    wram[0x1234] = 0x11;
    wram[0x1235] = 0x11;
    wram[0x1236] = 0x11;
    wram[0x1237] = 0x11;
    wram[0x1238] = 0x11;
    wram[0x1239] = 0x11;
    wram[0x123A] = 0x11;
    wram[0x123B] = 0x11;

    memory->Write8Bit(0x4300 | eDma_DMAPx, 0x10); // AtoB, B increment mode 0, A decrement.
    memory->Write8Bit(0x4300 | eDma_BBADx, 0x18); // Write to 2118/2119
    memory->Write8Bit(0x4300 | eDma_A1TxL, 0x3B); // Read from 0x7E123B
    memory->Write8Bit(0x4300 | eDma_A1TxH, 0x12);
    memory->Write8Bit(0x4300 | eDma_A1Bx, 0x7E);
    memory->Write8Bit(0x4300 | eDma_DASxL, 0x08); // Write 8 bytes
    memory->Write8Bit(0x4300 | eDma_DASxH, 0x00); // Write 8 bytes
    memory->Write8Bit(eRegMDMAEN, 0x01); // Start DMA channel 1

    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAL), 8);
    EXPECT_EQ(fakePpu.bytesWritten.count(eRegVMDATAH), 0);

    auto range = fakePpu.bytesWritten.equal_range(eRegVMDATAL);
    for (auto it = range.first; it != range.second; ++it)
    {
        EXPECT_EQ(it->second, 0x11);
    }

    // Byte count should be zero.
    EXPECT_EQ(memory->Read8Bit(0x4305), 0);
    EXPECT_EQ(memory->Read8Bit(0x4306), 0);

    // A bus increment.
    EXPECT_EQ(memory->Read8Bit(0x4302), 0x33);
}