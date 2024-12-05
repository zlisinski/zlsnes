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
};


MemoryTest::MemoryTest()
{
    memory = new Memory();
    timer = new Timer();
    memory->SetTimer(timer);
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