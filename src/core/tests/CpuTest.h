#pragma once

#include <gtest/gtest.h>

class Cpu;
class Memory;

class CpuTest : public ::testing::Test
{
protected:
    CpuTest();
    ~CpuTest() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();

    Cpu *cpu;
    Memory *memory_;

    uint8_t *memory;
};