#include <gtest/gtest.h>

// Include the mocks first so they override subsequent includes.
#include "mock/Apu.h"
#include "mock/Memory.h"
#include "mock/Interrupt.h"

#include "Timer.h"


class TimerTest : public ::testing::Test
{
protected:
    TimerTest();
    ~TimerTest() override;

    void SetUp() override;
    void TearDown() override;

    // Used for testing private methods.
    void SetClockCounter(uint32_t value) {timer->clockCounter = value;}
    void SetHCount(uint16_t value) {timer->hCount = value;}
    void SetVCount(uint16_t value) {timer->vCount = value;}
    void WriteRegister(EIORegisters ioReg, uint8_t byte) {timer->WriteRegister(ioReg, byte);}

    Interrupt *interrupts;
    Memory *memory;
    Timer *timer;
};


TimerTest::TimerTest()
{
    memory = new Memory();
    interrupts = new Interrupt();
    timer = new Timer(memory, interrupts);
}

TimerTest::~TimerTest()
{
    delete timer;
    delete interrupts;
    delete memory;
}

void TimerTest::SetUp()
{

}

void TimerTest::TearDown()
{

}


TEST_F(TimerTest, TEST_VBlankInterrupt_On_Timer)
{
    // Dont set the interrupt if NMI is disabled.
    EXPECT_EQ(interrupts->IsNmi(), false);
    SetVCount(224);
    SetHCount(341);
    SetClockCounter(1363);
    *memory->GetBytePtr(eRegNMITIMEN) = 0x00;
    timer->AddCycle(6);
    EXPECT_EQ(interrupts->IsNmi(), false);

    // Set the interrupt if NMI is enabled.
    EXPECT_EQ(interrupts->IsNmi(), false);
    SetVCount(224);
    SetHCount(341);
    SetClockCounter(1363);
    *memory->GetBytePtr(eRegNMITIMEN) = 0x80;
    timer->AddCycle(6);
    EXPECT_EQ(interrupts->IsNmi(), true);
}


TEST_F(TimerTest, TEST_VBlankInterrupt_On_NMITIMEN_Change)
{
    // Set the interrupt when NMITIMEN changes.
    EXPECT_EQ(interrupts->IsNmi(), false);
    *memory->GetBytePtr(eRegRDNMI) = 0x80;
    *memory->GetBytePtr(eRegNMITIMEN) = 0x00;
    WriteRegister(eRegNMITIMEN, 0x80);
    EXPECT_EQ(interrupts->IsNmi(), true);

    // Dont set the interrupt when NMITIMEN doesnt change.
    interrupts->ClearNmi();
    EXPECT_EQ(interrupts->IsNmi(), false);
    *memory->GetBytePtr(eRegRDNMI) = 0x80;
    WriteRegister(eRegNMITIMEN, 0x80);
    EXPECT_EQ(interrupts->IsNmi(), false);

    // Dont set the interrupt when not in VBlank.
    interrupts->ClearNmi();
    EXPECT_EQ(interrupts->IsNmi(), false);
    *memory->GetBytePtr(eRegRDNMI) = 0x00;
    *memory->GetBytePtr(eRegNMITIMEN) = 0x00;
    WriteRegister(eRegNMITIMEN, 0x80);
    EXPECT_EQ(interrupts->IsNmi(), false);
}