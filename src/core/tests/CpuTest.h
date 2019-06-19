#pragma once

#include <gtest/gtest.h>
#include "../Cpu.h"

class Memory;

class CpuTest : public ::testing::Test
{
protected:
    CpuTest();
    ~CpuTest() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();
    uint32_t GetPC() {return Make24Bit(cpu->reg.pb, cpu->reg.pc);}

    // Used for testing private methods.
    bool IsAccumulator8Bit() {return cpu->IsAccumulator8Bit();}
    bool IsAccumulator16Bit() {return cpu->IsAccumulator16Bit();}
    bool IsIndex8Bit() {return cpu->IsIndex8Bit();}
    bool IsIndex16Bit() {return cpu->IsIndex16Bit();}
    void LoadRegister8Bit(uint16_t *dest, uint16_t value) {cpu->LoadRegister8Bit(dest, value);}
    void LoadRegister16Bit(uint16_t *dest, uint16_t value) {cpu->LoadRegister16Bit(dest, value);}

    Cpu *cpu;
    Memory *memory_;

    uint8_t *memory;
};