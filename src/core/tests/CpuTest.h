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
    uint16_t GetOpAbsolute() {return cpu->GetOpAbsolute();}
    uint16_t GetOpAbsoluteIndexedX() {return cpu->GetOpAbsoluteIndexedX();}
    uint16_t GetOpAbsoluteIndexedY() {return cpu->GetOpAbsoluteIndexedY();}
    uint32_t GetOpAbsoluteIndirect() {return cpu->GetOpAbsoluteIndirect();}
    uint32_t GetOpAbsoluteIndexedIndirect() {return cpu->GetOpAbsoluteIndexedIndirect();}
    uint16_t GetOpAccumulator() {return cpu->GetOpAccumulator();}
    uint16_t GetOpDirect() {return cpu->GetOpDirect();}
    uint16_t GetOpDirectIndexedX() {return cpu->GetOpDirectIndexedX();}
    uint16_t GetOpDirectIndexedY() {return cpu->GetOpDirectIndexedY();}
    uint16_t GetOpDirectIndirect() {return cpu->GetOpDirectIndirect();}
    uint16_t GetOpDirectIndirectLong() {return cpu->GetOpDirectIndirectLong();}
    uint16_t GetOpDirectIndexedIndirect() {return cpu->GetOpDirectIndexedIndirect();}
    uint16_t GetOpDirectIndirectIndexed() {return cpu->GetOpDirectIndirectIndexed();}
    uint16_t GetOpDirectIndirectLongIndexed() {return cpu->GetOpDirectIndirectLongIndexed();}
    uint16_t GetOpAbsoluteLong() {return cpu->GetOpAbsoluteLong();}
    uint16_t GetOpAbsoluteLongIndexedX() {return cpu->GetOpAbsoluteLongIndexedX();}
    uint16_t GetOpStackRelative() {return cpu->GetOpStackRelative();}
    uint16_t GetOpStackRelativeIndirectIndexed() {return cpu->GetOpStackRelativeIndirectIndexed();}
    void LoadRegister(uint16_t *dest, uint16_t value, bool is16Bit) {cpu->LoadRegister(dest, value, is16Bit);}

    Cpu *cpu;
    Memory *memory_;

    uint8_t *memory;
};