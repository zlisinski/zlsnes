#include <gtest/gtest.h>
#include <unordered_map>

#include "Memory.h"
#include "Timer.h"


class MemoryTest : public ::testing::Test
{
protected:
    MemoryTest();
    ~MemoryTest() override;

    void SetUp() override;
    void TearDown() override;

    Memory *memory;
    Timer *timer;

    uint8_t *wram;
    uint8_t *ioPorts21;
    uint8_t *ioPorts40;
    uint8_t *ioPorts42;
    uint8_t *ioPorts43;
};


MemoryTest::MemoryTest()
{
    memory = new Memory();
    timer = new Timer();
    memory->SetTimer(timer);

    wram = &memory->wram[0];
    ioPorts21 = &memory->ioPorts21[0];
    ioPorts40 = &memory->ioPorts40[0];
    ioPorts42 = &memory->ioPorts42[0];
    ioPorts43 = &memory->ioPorts43[0];
}

MemoryTest::~MemoryTest()
{
    delete memory;
    delete timer;
}

void MemoryTest::SetUp()
{

}

void MemoryTest::TearDown()
{

}

TEST_F(MemoryTest, TEST_Read8Bit_uint32)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
}

TEST_F(MemoryTest, TEST_Read8Bit_Address)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    EXPECT_EQ(memory->Read8Bit(Address(0x7EFFFF)), 0xAB);
}

TEST_F(MemoryTest, TEST_ReadRaw8Bit)
{
    // TODO: Change this to a memory location that is specially handled by Read8Bit() so we can test that it is bypassing.
    memory->Write8Bit(0x7EFFFF, 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
}

TEST_F(MemoryTest, TEST_Read16Bit_uint32)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7F0000, 0x89);
    EXPECT_EQ(memory->Read16Bit(0x7EFFFF), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read16Bit_Address)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7F0000, 0x89);
    EXPECT_EQ(memory->Read16Bit(Address(0x7EFFFF)), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read24Bit_uint32)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7F0000, 0x89);
    memory->Write8Bit(0x7F0001, 0x67);
    EXPECT_EQ(memory->Read24Bit(0x7EFFFF), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Read24Bit_Address)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7F0000, 0x89);
    memory->Write8Bit(0x7F0001, 0x67);
    EXPECT_EQ(memory->Read24Bit(Address(0x7EFFFF)), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Read16BitWrapBank_uint32)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7E0000, 0x89);
    EXPECT_EQ(memory->Read16BitWrapBank(0x7E, 0xFFFF), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read16BitWrapBank_Address)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7E0000, 0x89);
    EXPECT_EQ(memory->Read16BitWrapBank(Address(0x7EFFFF)), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read24BitWrapBank_uint32)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7E0000, 0x89);
    memory->Write8Bit(0x7E0001, 0x67);
    EXPECT_EQ(memory->Read24BitWrapBank(0x7E, 0xFFFF), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Read24BitWrapBank_Address)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    memory->Write8Bit(0x7E0000, 0x89);
    memory->Write8Bit(0x7E0001, 0x67);
    EXPECT_EQ(memory->Read24BitWrapBank(Address(0x7EFFFF)), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Write8Bit_uint32)
{
    memory->Write8Bit(0x7EFFFF, 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
}

TEST_F(MemoryTest, TEST_Write8Bit_Address)
{
    memory->Write8Bit(Address(0x7EFFFF), 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
}

/*TEST_F(MemoryTest, TEST_Write16Bit_uint32)
{
    memory->Write16Bit(0x7EFFFF, 0x89AB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7F0000), 0x89);
}*/

TEST_F(MemoryTest, TEST_Write16Bit_Address)
{
    memory->Write16Bit(Address(0x7EFFFF), 0x89AB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7F0000), 0x89);
}

TEST_F(MemoryTest, TEST_Write16BitWrapBank_Address)
{
    memory->Write16BitWrapBank(Address(0x7EFFFF), 0x89AB);
    EXPECT_EQ(memory->Read8Bit(0x7EFFFF), 0xAB);
    EXPECT_EQ(memory->Read8Bit(0x7E0000), 0x89);
}


TEST_F(MemoryTest, TEST_WRam_Mirror)
{
    // Writes to WRAM should be mirrored into other banks.
    memory->Write8Bit(0x7E0001, 0x99);

    // Writes to mirrored area should be applied to WRAM.
    memory->Write8Bit(0x00000A, 0x77);

    EXPECT_EQ(memory->Read8Bit(0x7E0001), 0x99);
    EXPECT_EQ(memory->Read8Bit(0x7E000A), 0x77);
    EXPECT_EQ(wram[0x0001], 0x99);
    EXPECT_EQ(wram[0x000A], 0x77);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x0001)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x0001)), 0x99) << i + 0x80;
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x000A)), 0x77) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x000A)), 0x77) << i + 0x80;
    }
}


/*TEST_F(MemoryTest, TEST_ioPorts21_Mirror)
{
    // Writes to 21xx ports should be mirrored into other banks.
    memory->Write8Bit(0x002180, 0x99);

    EXPECT_EQ(ioPorts21[0x80], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x2180)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x2180)), 0x99) << i + 0x80;
    }
}


TEST_F(MemoryTest, TEST_ioPorts40_Mirror)
{
    // Writes to 40xx ports should be mirrored into other banks.
    memory->Write8Bit(0x004016, 0x99);

    EXPECT_EQ(ioPorts40[0x16], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x4016)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x4016)), 0x99) << i + 0x80;
    }
}


TEST_F(MemoryTest, TEST_ioPorts42_Mirror)
{
    // Writes to 42xx ports should be mirrored into other banks.
    memory->Write8Bit(0x004200, 0x99);

    EXPECT_EQ(ioPorts42[0x00], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x4200)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x4200)), 0x99) << i + 0x80;
    }
}


TEST_F(MemoryTest, TEST_ioPorts43_Mirror)
{
    // Writes to 43xx ports should be mirrored into other banks.
    memory->Write8Bit(0x004380, 0x99);

    EXPECT_EQ(ioPorts43[0x80], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x4380)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x4380)), 0x99) << i + 0x80;
    }
}*/


TEST_F(MemoryTest, TEST_WRAM_read_write_ports)
{
    // Write address registers
    memory->Write8Bit(eRegWMADDH, 0x01);
    memory->Write8Bit(eRegWMADDM, 0x23);
    memory->Write8Bit(eRegWMADDL, 0x45);

    // Write some data
    memory->Write8Bit(eRegWMDATA, 0xAB);
    memory->Write8Bit(eRegWMDATA, 0xCD);

    // Values should be written to wrma.
    EXPECT_EQ(wram[0x12345], 0xAB);
    EXPECT_EQ(wram[0x12346], 0xCD);

    // There is an interal counter, so reads won't read the last bytes written
    EXPECT_EQ(memory->Read8Bit(eRegWMDATA), 0x00);
    EXPECT_EQ(memory->Read8Bit(eRegWMDATA), 0x00);

    // Reset the address back to the start
    memory->Write8Bit(eRegWMADDH, 0x01);
    memory->Write8Bit(eRegWMADDM, 0x23);
    memory->Write8Bit(eRegWMADDL, 0x45);

    // Now the values written can be read.
    EXPECT_EQ(memory->Read8Bit(eRegWMDATA), 0xAB);
    EXPECT_EQ(memory->Read8Bit(eRegWMDATA), 0xCD);
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


TEST_F(MemoryTest, TEST_Dma1)
{
    FakeIo fakePpu;
    memory->AttachIoRegister(eRegVMDATAL, &fakePpu);
    memory->AttachIoRegister(eRegVMDATAH, &fakePpu);

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

    ioPorts43[0x00] = 0x01; // AtoB, B increment mode 1, +1 A increment.
    ioPorts43[0x01] = 0x18; // Write to 2118/2119
    ioPorts43[0x02] = 0x34; // Read from 0x7E1234
    ioPorts43[0x03] = 0x12;
    ioPorts43[0x04] = 0x7E;
    ioPorts43[0x05] = 0x10; // Write 16 bytes
    ioPorts43[0x06] = 0x00; // Write 16 bytes
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