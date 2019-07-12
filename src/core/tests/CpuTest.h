#pragma once

#include <gtest/gtest.h>
#include "../Bytes.h"
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
    uint32_t GetPC() {return Bytes::Make24Bit(cpu->reg.pb, cpu->reg.pc);}

    // Used for testing private methods.
    bool IsAccumulator8Bit() {return cpu->IsAccumulator8Bit();}
    bool IsAccumulator16Bit() {return cpu->IsAccumulator16Bit();}
    bool IsIndex8Bit() {return cpu->IsIndex8Bit();}
    bool IsIndex16Bit() {return cpu->IsIndex16Bit();}
    void LoadRegister8Bit(uint16_t *dest, uint16_t value) {cpu->LoadRegister8Bit(dest, value);}
    void LoadRegister16Bit(uint16_t *dest, uint16_t value) {cpu->LoadRegister16Bit(dest, value);}
    void Push8Bit(uint8_t value) {cpu->Push8Bit(value);}
    uint8_t Pop8Bit() {return cpu->Pop8Bit();}

    Cpu *cpu;
    Memory *memory_;

    uint8_t *memory;
};