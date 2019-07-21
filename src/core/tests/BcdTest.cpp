#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "BcdTest.h"
#include "../Bcd.h"

BcdTest::BcdTest()
{

}

BcdTest::~BcdTest()
{

}

void BcdTest::SetUp()
{

}

void BcdTest::TearDown()
{

}

TEST_F(BcdTest, TEST_Add)
{
    char message[256];

    // Testing every permutation takes way too long (> 10 minutes), so only run a random sample.

    /*for (int i = 0; i < 10000; i++)
    {
        for (int j = 0; j < 10000; j++)
        {
            uint32_t result = Bcd::Add(Bcd::ToBcd(i), Bcd::ToBcd(j));

            snprintf(message, sizeof(message), "i=%d j=%d", i, j);
            SCOPED_TRACE(message);

            printf("%s = %04X\n", message, result);
            ASSERT_EQ(result, Bcd::ToBcd(i + j));
        }
    }*/

    srand(time(NULL));

    for (int i = 0; i < 100000; i++)
    {
        uint16_t a = rand() % 10000;
        uint16_t b = rand() % 10000;
        uint32_t result = Bcd::Add(Bcd::ToBcd(a), Bcd::ToBcd(b));

        snprintf(message, sizeof(message), "a=%d b=%d", a, b);
        SCOPED_TRACE(message);

        //printf("%s = %04X\n", message, result);
        ASSERT_EQ(result, Bcd::ToBcd(a + b));
    }
}

TEST_F(BcdTest, TEST_Subtract)
{
    char message[256];

    // Testing every permutation takes way too long (> 10 minutes), so only run a random sample.

    /*for (int i = 0; i < 10000; i++)
    {
        for (int j = 0; j < 10000; j++)
        {
            uint32_t result = Bcd::Subtract(Bcd::ToBcd(i), Bcd::ToBcd(j));

            uint32_t expected;
            if (i < j)
                expected = Bcd::ToBcd((i - j) + 10000);
            else
                expected = 0x00010000 | Bcd::ToBcd(i - j);

            snprintf(message, sizeof(message), "i=%d j=%d", i, j);
            SCOPED_TRACE(message);

            printf("%s = %05X (%05X)\n", message, result, expected);
            ASSERT_EQ(result, expected);
        }
    }*/

    srand(time(NULL));

    for (int i = 0; i < 100000; i++)
    {
        uint16_t a = rand() % 10000;
        uint16_t b = rand() % 10000;
        uint32_t result = Bcd::Subtract(Bcd::ToBcd(a), Bcd::ToBcd(b));

        uint32_t expected;
        if (a < b)
            expected = Bcd::ToBcd((a - b) + 10000);
        else
            expected = 0x00010000 | Bcd::ToBcd(a - b);

        snprintf(message, sizeof(message), "a=%d b=%d", a, b);
        SCOPED_TRACE(message);

        //printf("%s = %05X (%05X)\n", message, result, expected);
        ASSERT_EQ(result, expected);
    }
}