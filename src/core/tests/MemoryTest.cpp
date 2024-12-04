#include "MemoryTest.h"

MemoryTest::MemoryTest()
{
    memory = new Memory();
}

MemoryTest::~MemoryTest()
{
    delete memory;
}

void MemoryTest::SetUp()
{
    ResetState();
}

void MemoryTest::TearDown()
{

}

void MemoryTest::ResetState()
{
    memory->ClearMemory();
    mem = memory->GetBytePtr(0);
}


TEST_F(MemoryTest, TEST_Read8Bit_uint32)
{
    mem[0x12FFFF] = 0xAB;
    EXPECT_EQ(memory->Read8Bit(0x12FFFF), 0xAB);
}

TEST_F(MemoryTest, TEST_Read8Bit_Address)
{
    mem[0x12FFFF] = 0xAB;
    EXPECT_EQ(memory->Read8Bit(Address(0x12FFFF)), 0xAB);
}

TEST_F(MemoryTest, TEST_ReadRaw8Bit)
{
    // TODO: Change this to a memory location that is specially handled by Read8Bit() so we can test that it is bypassing.
    mem[0x12FFFF] = 0xAB;
    EXPECT_EQ(memory->Read8Bit(0x12FFFF), 0xAB);
}

TEST_F(MemoryTest, TEST_Read16Bit_uint32)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x130000] = 0x89;
    EXPECT_EQ(memory->Read16Bit(0x12FFFF), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read16Bit_Address)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x130000] = 0x89;
    EXPECT_EQ(memory->Read16Bit(Address(0x12FFFF)), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read24Bit_uint32)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x130000] = 0x89;
    mem[0x130001] = 0x67;
    EXPECT_EQ(memory->Read24Bit(0x12FFFF), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Read24Bit_Address)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x130000] = 0x89;
    mem[0x130001] = 0x67;
    EXPECT_EQ(memory->Read24Bit(Address(0x12FFFF)), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Read16BitWrapBank_uint32)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x120000] = 0x89;
    EXPECT_EQ(memory->Read16BitWrapBank(0x12, 0xFFFF), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read16BitWrapBank_Address)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x120000] = 0x89;
    EXPECT_EQ(memory->Read16BitWrapBank(Address(0x12FFFF)), 0x89AB);
}

TEST_F(MemoryTest, TEST_Read24BitWrapBank_uint32)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x120000] = 0x89;
    mem[0x120001] = 0x67;
    EXPECT_EQ(memory->Read24BitWrapBank(0x12, 0xFFFF), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Read24BitWrapBank_Address)
{
    mem[0x12FFFF] = 0xAB;
    mem[0x120000] = 0x89;
    mem[0x120001] = 0x67;
    EXPECT_EQ(memory->Read24BitWrapBank(Address(0x12FFFF)), 0x6789AB);
}

TEST_F(MemoryTest, TEST_Write8Bit_uint32)
{
    memory->Write8Bit(0x12FFFF, 0xAB);
    EXPECT_EQ(mem[0x12FFFF], 0xAB);
}

TEST_F(MemoryTest, TEST_Write8Bit_Address)
{
    memory->Write8Bit(Address(0x12FFFF), 0xAB);
    EXPECT_EQ(mem[0x12FFFF], 0xAB);
}

/*TEST_F(MemoryTest, TEST_Write16Bit_uint32)
{
    memory->Write16Bit(0x12FFFF, 0x89AB);
    EXPECT_EQ(mem[0x12FFFF], 0xAB);
    EXPECT_EQ(mem[0x130000], 0x89);
}*/

TEST_F(MemoryTest, TEST_Write16Bit_Address)
{
    memory->Write16Bit(Address(0x12FFFF), 0x89AB);
    EXPECT_EQ(mem[0x12FFFF], 0xAB);
    EXPECT_EQ(mem[0x130000], 0x89);
}

TEST_F(MemoryTest, TEST_Write16BitWrapBank_Address)
{
    memory->Write16BitWrapBank(Address(0x12FFFF), 0x89AB);
    EXPECT_EQ(mem[0x12FFFF], 0xAB);
    EXPECT_EQ(mem[0x120000], 0x89);
}