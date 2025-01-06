#include <gtest/gtest.h>

// Include the mocks first so they override subsequent includes.
#include "mock/Memory.h"
#include "mock/Timer.h"

#include "Ppu.h"

class PpuTest : public ::testing::Test
{
protected:
    PpuTest();
    ~PpuTest() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();

    // Used for testing private methods.
    uint16_t GetBgHOffset(int i) {return ppu->bgHOffset[i];}
    uint16_t GetBgVOffset(int i) {return ppu->bgVOffset[i];}
    uint8_t GetCgramData(uint16_t addr) {return ppu->cgram[addr];}
    uint8_t GetOamData(uint16_t addr) {return ppu->oam[addr];}
    uint16_t TranslateVramAddress(uint16_t addr, uint8_t translate) {return ppu->TranslateVramAddress(addr, translate);}

    Ppu *ppu;
    Memory *memory;
    Timer *timer;
};

PpuTest::PpuTest()
{
    memory = new Memory();
    timer = new Timer();
    ppu = new Ppu(memory, timer, nullptr);
}

PpuTest::~PpuTest()
{
    delete ppu;
    delete timer;
    delete memory;
}

void PpuTest::SetUp()
{

}

void PpuTest::TearDown()
{

}


TEST_F(PpuTest, TEST_BGHOFS_Write_Twice)
{
    // Offsets are limited to 10 bits.

    ppu->WriteRegister(eRegBG1HOFS, 0x34);
    ppu->WriteRegister(eRegBG1HOFS, 0x12);
    EXPECT_EQ(GetBgHOffset(0), 0x1234 & 0x3FF);

    ppu->WriteRegister(eRegBG2HOFS, 0x78);
    ppu->WriteRegister(eRegBG2HOFS, 0x56);
    EXPECT_EQ(GetBgHOffset(1), 0x5678 & 0x3FF);

    ppu->WriteRegister(eRegBG3HOFS, 0xBC);
    ppu->WriteRegister(eRegBG3HOFS, 0x9A);
    EXPECT_EQ(GetBgHOffset(2), 0x9ABC & 0x3FF);

    ppu->WriteRegister(eRegBG4HOFS, 0xF0);
    ppu->WriteRegister(eRegBG4HOFS, 0xDE);
    EXPECT_EQ(GetBgHOffset(3), 0xDEF0 & 0x3FF);
}


TEST_F(PpuTest, TEST_BGVOFS_Write_Twice)
{
    // Offsets are limited to 10 bits.

    ppu->WriteRegister(eRegBG1VOFS, 0x34);
    ppu->WriteRegister(eRegBG1VOFS, 0x12);
    EXPECT_EQ(GetBgVOffset(0), 0x1234 & 0x3FF);

    ppu->WriteRegister(eRegBG2VOFS, 0x78);
    ppu->WriteRegister(eRegBG2VOFS, 0x56);
    EXPECT_EQ(GetBgVOffset(1), 0x5678 & 0x3FF);

    ppu->WriteRegister(eRegBG3VOFS, 0xBC);
    ppu->WriteRegister(eRegBG3VOFS, 0x9A);
    EXPECT_EQ(GetBgVOffset(2), 0x9ABC & 0x3FF);

    ppu->WriteRegister(eRegBG4VOFS, 0xF0);
    ppu->WriteRegister(eRegBG4VOFS, 0xDE);
    EXPECT_EQ(GetBgVOffset(3), 0xDEF0 & 0x3FF);
}


TEST_F(PpuTest, TEST_CGDATA_Write_Twice)
{
    // This is a word address, so double it (0x20) when reading memory.
    ppu->WriteRegister(eRegCGADD, 0x10);

    // Test that both bytes are only written to cgram after the second write to the register.
    ppu->WriteRegister(eRegCGDATA, 0x12);
    EXPECT_EQ(GetCgramData(0x20), 0);
    ppu->WriteRegister(eRegCGDATA, 0x34);
    EXPECT_EQ(GetCgramData(0x20), 0x12);
    EXPECT_EQ(GetCgramData(0x21), 0x34);

    // Test that changing the address after one byte is read will lose that value.
    ppu->WriteRegister(eRegCGADD, 0x20);
    ppu->WriteRegister(eRegCGDATA, 0x12);
    EXPECT_EQ(GetCgramData(0x40), 0);
    // Change the address after one byte written. First byte (0x12) should be lost.
    ppu->WriteRegister(eRegCGADD, 0x30);
    ppu->WriteRegister(eRegCGDATA, 0x56);
    EXPECT_EQ(GetCgramData(0x60), 0);
    ppu->WriteRegister(eRegCGDATA, 0x78);
    EXPECT_EQ(GetCgramData(0x60), 0x56);
    EXPECT_EQ(GetCgramData(0x61), 0x78);
    EXPECT_EQ(GetCgramData(0x40), 0);
}


TEST_F(PpuTest, TEST_OAMDATA_Write_Twice)
{
    // This is a word address, so double it (0x20) when reading memory.
    ppu->WriteRegister(eRegOAMADDL, 0x10);
    ppu->WriteRegister(eRegOAMADDH, 0x00);

    // Test that both bytes are only written to oam after the second write to the register.
    ppu->WriteRegister(eRegOAMDATA, 0x12);
    EXPECT_EQ(GetOamData(0x20), 0);
    ppu->WriteRegister(eRegOAMDATA, 0x34);
    EXPECT_EQ(GetOamData(0x20), 0x12);
    EXPECT_EQ(GetOamData(0x21), 0x34);

    // Test that changing the address after one byte is read will lose that value.
    ppu->WriteRegister(eRegOAMADDL, 0x20);
    ppu->WriteRegister(eRegOAMDATA, 0x12);
    EXPECT_EQ(GetOamData(0x40), 0);
    // Change the address after one byte written. First byte (0x12) should be lost.
    ppu->WriteRegister(eRegOAMADDL, 0x30);
    ppu->WriteRegister(eRegOAMDATA, 0x56);
    EXPECT_EQ(GetOamData(0x60), 0);
    ppu->WriteRegister(eRegOAMDATA, 0x78);
    EXPECT_EQ(GetOamData(0x60), 0x56);
    EXPECT_EQ(GetOamData(0x61), 0x78);
    EXPECT_EQ(GetOamData(0x40), 0);

    // Writing to the high table should write immediately
    ppu->WriteRegister(eRegOAMADDL, 0x00);
    ppu->WriteRegister(eRegOAMADDH, 0x01);
    ppu->WriteRegister(eRegOAMDATA, 0x56);
    EXPECT_EQ(GetOamData(0x200), 0x56);

    // Writing to > 0x21F should mirror.
    ppu->WriteRegister(eRegOAMADDL, 0x7F); // 0x2FE, mirrored 0x21E
    ppu->WriteRegister(eRegOAMADDH, 0x01);
    ppu->WriteRegister(eRegOAMDATA, 0xAB);
    EXPECT_EQ(GetOamData(0x21E), 0xAB);
}


TEST_F(PpuTest, TEST_TranslateVramAddress)
{
    uint16_t addr = 0x1234;
    uint16_t ret = TranslateVramAddress(addr, 0);
    EXPECT_EQ(ret, addr);

    ret = TranslateVramAddress(addr, 1);
    EXPECT_EQ(ret, 0x12A1);

    ret = TranslateVramAddress(addr, 2);
    EXPECT_EQ(ret, 0x13A0);

    ret = TranslateVramAddress(addr, 3);
    EXPECT_EQ(ret, 0x11A4);
}


TEST_F(PpuTest, TEST_Multiplication)
{
    // 0 * 10 = 0
    ppu->WriteRegister(eRegM7B, 10);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYH), 0);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYM), 0);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYL), 0);

    // Writing one byte uses the latch value, which is shared between M7A and M7B.
    // 0x800A(-32758) * 10 = -327580
    ppu->WriteRegister(eRegM7A, 0x80);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYH), 0xFB);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYM), 0x00);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYL), 0x64);

    // Write second byte.
    // 0x0080(128) * 10 = 1280
    ppu->WriteRegister(eRegM7A, 0x00);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYH), 0x00);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYM), 0x05);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYL), 0x00);

    // M7B always uses the last byte written, not the full 16-bit value.
    // 0x0080(128) * -1 = -128
    ppu->WriteRegister(eRegM7B, 0xFF);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYH), 0xFF);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYM), 0xFF);
    EXPECT_EQ(ppu->ReadRegister(eRegMPYL), 0x80);
}