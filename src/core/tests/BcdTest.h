#pragma once

#include <gtest/gtest.h>

class BcdTest : public ::testing::Test
{
protected:
    BcdTest();
    ~BcdTest() override;

    void SetUp() override;
    void TearDown() override;
};