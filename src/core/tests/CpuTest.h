#pragma once

#include <gtest/gtest.h>
#include "../Bytes.h"
#include "../Cpu.h"

class Memory;
class QString;
class QJsonObject;

class CpuTest : public ::testing::Test
{
protected:
    CpuTest();
    ~CpuTest() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();
    void RunInstructionTest(const QString &opcodeName, const QString &opcode, bool emulationMode);
    void FormatData(const QJsonObject &obj, QString &str);

    uint32_t GetPC() {return Bytes::Make24Bit(cpu->reg.pb, cpu->reg.pc);}

    // Used for testing private methods.
    bool IsAccumulator8Bit() {return cpu->IsAccumulator8Bit();}
    bool IsAccumulator16Bit() {return cpu->IsAccumulator16Bit();}
    bool IsIndex8Bit() {return cpu->IsIndex8Bit();}
    bool IsIndex16Bit() {return cpu->IsIndex16Bit();}
    template <typename T> void LoadRegister(T *dest, T value) {cpu->LoadRegister(dest, value);}
    void Push8Bit(uint8_t value) {cpu->Push8Bit(value);}
    uint8_t Pop8Bit() {return cpu->Pop8Bit();}

    Cpu *cpu;
    Memory *memory_;

    uint8_t *memory;
};