#pragma once

#include <gtest/gtest.h>
#include "../AddressMode.h"
#include "../Bytes.h"

class Cpu;
class Memory;
class Timer;

class AddressModeTest : public ::testing::Test
{
protected:
    AddressModeTest();
    ~AddressModeTest() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();
    uint32_t GetPC() {return Bytes::Make24Bit(cpu->reg.pb, cpu->reg.pc);}

    Cpu *cpu;
    Memory *memory_;
    Timer *timer;

    uint8_t *memory;
};