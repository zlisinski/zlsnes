#include <gtest/gtest.h>

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


TEST_F(MemoryTest, TEST_ioPorts21_Mirror)
{
    // Writes to 21xx ports should be mirrored into other banks.
    memory->Write8Bit(0x002184, 0x99);

    EXPECT_EQ(ioPorts21[0x84], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x2184)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x2184)), 0x99) << i + 0x80;
    }
}


TEST_F(MemoryTest, TEST_ioPorts40_Mirror)
{
    // Writes to 40xx ports should be mirrored into other banks.
    memory->Write8Bit(0x004018, 0x99);

    EXPECT_EQ(ioPorts40[0x18], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x4018)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x4018)), 0x99) << i + 0x80;
    }
}


TEST_F(MemoryTest, TEST_ioPorts42_Mirror)
{
    // Writes to 42xx ports should be mirrored into other banks.
    memory->Write8Bit(0x00420E, 0x99);

    EXPECT_EQ(ioPorts42[0x0E], 0x99);

    for (int i = 0; i < 0x40; i++)
    {
        EXPECT_EQ(memory->Read8Bit(Address(i, 0x420E)), 0x99) << i;
        EXPECT_EQ(memory->Read8Bit(Address(i + 0x80, 0x420E)), 0x99) << i + 0x80;
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
}