#pragma once

#include <gtest/gtest.h>
#include "../Memory.h"

class Timer;

class MemoryTest : public ::testing::Test
{
protected:
    MemoryTest();
    ~MemoryTest() override;

    void SetUp() override;
    void TearDown() override;

    void ResetState();

    Memory *memory;
    Timer *timer;
    uint8_t *mem;
};