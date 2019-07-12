#include <stdint.h>

#include "CpuTest.h"
#include "../Memory.h"
#include "../Zlsnes.h"

const uint16_t A_VALUE = 0x1234;
const uint16_t X_VALUE = 0x5678;
const uint16_t Y_VALUE = 0x9ABC;
const uint16_t D_VALUE = 0xDEF0;
const uint8_t P_VALUE = 0x00;
const uint8_t DB_VALUE = 0x12;
const uint8_t PB_VALUE = 0x34;
const uint16_t SP_VALUE = 0xFFFF;


CpuTest::CpuTest()
{
    memory_ = new Memory();
    cpu = new Cpu(memory_);
}

CpuTest::~CpuTest()
{
    delete cpu;
    delete memory_;
}

void CpuTest::SetUp()
{
    ResetState();
}

void CpuTest::TearDown()
{

}

void CpuTest::ResetState()
{
    cpu->reg.a = A_VALUE;
    cpu->reg.x = X_VALUE;
    cpu->reg.y = Y_VALUE;
    cpu->reg.d = D_VALUE;
    cpu->reg.p = P_VALUE;
    cpu->reg.db = DB_VALUE;
    cpu->reg.pb = PB_VALUE;
    cpu->reg.sp = SP_VALUE;
    cpu->reg.pc = 0;
    cpu->reg.emulationMode = false;

    memory_->ClearMemory();
    memory = memory_->GetBytePtr(0);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_LoadRegister8Bit)
{
    uint16_t dest;

    ResetState();
    dest = 0x1234;
    LoadRegister8Bit(&dest, 0);
    ASSERT_EQ(dest, 0x1200);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    dest = 0x1234;
    LoadRegister8Bit(&dest, 0x5678);
    ASSERT_EQ(dest, 0x1278);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    dest = 0x1234;
    LoadRegister8Bit(&dest, 0x80);
    ASSERT_EQ(dest, 0x1280);
    ASSERT_EQ(cpu->reg.flags.n, 1);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x80);

}

TEST_F(CpuTest, TEST_LoadRegister16Bit)
{
    uint16_t dest;

    ResetState();
    dest = 0;
    LoadRegister16Bit(&dest, 0x1234);
    ASSERT_EQ(dest, 0x1234);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    dest = 0xFFFF;
    LoadRegister16Bit(&dest, 0);
    ASSERT_EQ(dest, 0);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    dest = 0;
    LoadRegister16Bit(&dest, 0xFFFF);
    ASSERT_EQ(dest, 0xFFFF);
    ASSERT_EQ(cpu->reg.flags.n, 1);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x80);
}

TEST_F(CpuTest, TEST_IsXYBit)
{
    cpu->reg.emulationMode = false;
    cpu->reg.flags.m = 0;
    ASSERT_TRUE(IsAccumulator16Bit());
    ASSERT_FALSE(IsAccumulator8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.m = 0;
    ASSERT_FALSE(IsAccumulator16Bit());
    ASSERT_TRUE(IsAccumulator8Bit());

    cpu->reg.emulationMode = false;
    cpu->reg.flags.m = 1;
    ASSERT_FALSE(IsAccumulator16Bit());
    ASSERT_TRUE(IsAccumulator8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.m = 1;
    ASSERT_FALSE(IsAccumulator16Bit());
    ASSERT_TRUE(IsAccumulator8Bit());

    cpu->reg.emulationMode = false;
    cpu->reg.flags.x = 0;
    ASSERT_TRUE(IsIndex16Bit());
    ASSERT_FALSE(IsIndex8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.x = 0;
    ASSERT_FALSE(IsIndex16Bit());
    ASSERT_TRUE(IsIndex8Bit());

    cpu->reg.emulationMode = false;
    cpu->reg.flags.x = 1;
    ASSERT_FALSE(IsIndex16Bit());
    ASSERT_TRUE(IsIndex8Bit());

    cpu->reg.emulationMode = true;
    cpu->reg.flags.x = 1;
    ASSERT_FALSE(IsIndex16Bit());
    ASSERT_TRUE(IsIndex8Bit());
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_TAX)
{
    memory[GetPC()] = 0xAA;
    cpu->reg.a = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0xAA;
    cpu->reg.a = 0;
    cpu->reg.flags.x = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE));
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x12);
}

TEST_F(CpuTest, TEST_TAY)
{
    memory[GetPC()] = 0xA8;
    cpu->reg.a = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0xA8;
    cpu->reg.a = 0;
    cpu->reg.flags.x = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE));
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x12);
}

TEST_F(CpuTest, TEST_TSX)
{
    memory[GetPC()] = 0xBA;
    cpu->reg.sp = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0xBA;
    cpu->reg.sp = 0;
    cpu->reg.flags.x = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE));
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x12);
}

TEST_F(CpuTest, TEST_TXA)
{
    memory[GetPC()] = 0x8A;
    cpu->reg.x = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0x8A;
    cpu->reg.x = 0;
    cpu->reg.flags.m = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE));
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x22);
}

TEST_F(CpuTest, TEST_TXS)
{
    memory[GetPC()] = 0x9A;
    cpu->reg.x = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0); // No flags are set.
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    memory[GetPC()] = 0x9A;
    cpu->reg.x = 0;
    cpu->reg.emulationMode = true;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x0100); // High byte of sp is always 0x01 in emulation mode.
    ASSERT_EQ(cpu->reg.flags.z, 0); // No flags are set.
    ASSERT_EQ(cpu->reg.p, 0x00);
}

TEST_F(CpuTest, TEST_TXY)
{
    memory[GetPC()] = 0x9B;
    cpu->reg.x = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, 0);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0x9B;
    cpu->reg.x = 0;
    cpu->reg.flags.x = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE));
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x12);
}

TEST_F(CpuTest, TEST_TYA)
{
    memory[GetPC()] = 0x98;
    cpu->reg.y = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0x98;
    cpu->reg.y = 0;
    cpu->reg.flags.m = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE));
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x22);
}

TEST_F(CpuTest, TEST_TYX)
{
    memory[GetPC()] = 0xBB;
    cpu->reg.y = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0);
    ASSERT_EQ(cpu->reg.y, 0);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    memory[GetPC()] = 0xBB;
    cpu->reg.y = 0;
    cpu->reg.flags.x = 1;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE));
    ASSERT_EQ(cpu->reg.y, 0);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x12);
}

TEST_F(CpuTest, TEST_TCD)
{
    memory[GetPC()] = 0x5B;
    cpu->reg.a = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);
}

TEST_F(CpuTest, TEST_TCS)
{
    memory[GetPC()] = 0x1B;
    cpu->reg.a = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0); // No flags are set.
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    memory[GetPC()] = 0x1B;
    cpu->reg.a = 0;
    cpu->reg.emulationMode = true;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x0100); // High byte of sp is always 0x01 in emulation mode.
    ASSERT_EQ(cpu->reg.flags.z, 0); // No flags are set.
    ASSERT_EQ(cpu->reg.p, 0x00);
}

TEST_F(CpuTest, TEST_TDC)
{
    memory[GetPC()] = 0x7B;
    cpu->reg.d = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);
}

TEST_F(CpuTest, TEST_TSC)
{
    memory[GetPC()] = 0x3B;
    cpu->reg.sp = 0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_LDA_DirectIndexedIndirect)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xA1;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xA1;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_StackRelative)
{
    cpu->reg.sp = 0xFF10;
    memory[GetPC()] = 0xA3;
    memory[GetPC() + 1] = 0xFA;
    memory[0x00000A] = 0xEF;
    memory[0x00000B] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0xFF10);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.sp = 0xFF10;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xA3;
    memory[GetPC() + 1] = 0xFA;
    memory[0x00000A] = 0xEF;
    memory[0x00000B] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0xFF10);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xA5;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xEF;
    memory[0x000000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xA5;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xEF;
    memory[0x000000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirectLong)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xA7;
    memory[GetPC() + 1] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xA7;
    memory[GetPC() + 1] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_Immediate)
{
    memory[GetPC()] = 0xA9;
    memory[GetPC() + 1] = 0xEF;
    memory[GetPC() + 2] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xA9;
    memory[GetPC() + 1] = 0xEF;
    memory[GetPC() + 2] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_Absolute)
{
    memory[GetPC()] = 0xAD;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xAD;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteLong)
{
    memory[GetPC()] = 0xAF;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xAF;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirectIndexed)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xB1;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xB1;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirect)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xB2;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xB2;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_StackRelativeIndirectIndexed)
{
    cpu->reg.sp = 0xFF10;
    cpu->reg.y = 0x0050;
    memory[GetPC()] = 0xB3;
    memory[GetPC() + 1] = 0xFA;
    memory[0x00000A] = 0xF0;
    memory[0x00000B] = 0xFF;
    memory[0x130040] = 0xEF;
    memory[0x130041] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x0050);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0xFF10);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.sp = 0xFF10;
    cpu->reg.y = 0x0050;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xB3;
    memory[GetPC() + 1] = 0xFA;
    memory[0x00000A] = 0xF0;
    memory[0x00000B] = 0xFF;
    memory[0x130040] = 0xEF;
    memory[0x130041] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x0050);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0xFF10);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_DirectIndexedX)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xB5;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xEF;
    memory[0x000009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xB5;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xEF;
    memory[0x000009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_DirectIndirectLongIndexed)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xB7;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;
    memory[0x130006] = 0xEF;
    memory[0x130007] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xB7;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;
    memory[0x130006] = 0xEF;
    memory[0x130007] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteIndexedY)
{
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xB9;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.y = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xB9;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteIndexedX)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xBD;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xBD;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDA_AbsoluteLongIndexedX)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xBF;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xCDEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0xBF;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_LDX_Immediate)
{
    memory[GetPC()] = 0xA2;
    memory[GetPC() + 1] = 0xEF;
    memory[GetPC() + 2] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0xCDEF);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xA2;
    memory[GetPC() + 1] = 0xEF;
    memory[GetPC() + 2] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDX_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xA6;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xEF;
    memory[0x000000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0xCDEF);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xA6;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xEF;
    memory[0x000000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDX_Absolute)
{
    memory[GetPC()] = 0xAE;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0xCDEF);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xAE;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDX_DirectIndexedY)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xB6;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xEF;
    memory[0x000009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0xCDEF);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xB6;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xEF;
    memory[0x000009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDX_AbsoluteIndexedY)
{
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xBE;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0xCDEF);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.y = 0x000A;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xBE;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDY_Immediate)
{
    memory[GetPC()] = 0xA0;
    memory[GetPC() + 1] = 0xEF;
    memory[GetPC() + 2] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0xCDEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xA0;
    memory[GetPC() + 1] = 0xEF;
    memory[GetPC() + 2] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDY_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xA4;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xEF;
    memory[0x000000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0xCDEF);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xA4;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xEF;
    memory[0x000000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDY_Absolute)
{
    memory[GetPC()] = 0xAC;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0xCDEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xAC;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0xEF;
    memory[0x130000] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDY_DirectIndexedX)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xB4;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xEF;
    memory[0x000009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, 0xCDEF);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xB4;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xEF;
    memory[0x000009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_LDY_AbsoluteIndexedX)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xBC;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, 0xCDEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.x = 0x000A;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xBC;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0xEF;
    memory[0x130009] = 0xCD;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE) | 0xEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_STA_DirectIndexedIndirect)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x81;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x81;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STA_StackRelative)
{
    cpu->reg.sp = 0xFF10;
    memory[GetPC()] = 0x83;
    memory[GetPC() + 1] = 0xFA;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0xFF10);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x00000A], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x00000B], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.sp = 0xFF10;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x83;
    memory[GetPC() + 1] = 0xFA;
    memory[0x00000B] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0xFF10);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x00000A], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x00000B], 0x11);
}

TEST_F(CpuTest, TEST_STA_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0x85;
    memory[GetPC() + 1] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x00FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x000000], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x85;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x00FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x000000], 0x11);
}

TEST_F(CpuTest, TEST_STA_DirectIndirectLong)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0x87;
    memory[GetPC() + 1] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x87;
    memory[GetPC() + 1] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STA_Absolute)
{
    memory[GetPC()] = 0x8D;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x8D;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STA_AbsoluteLong)
{
    memory[GetPC()] = 0x8F;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x8F;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STA_DirectIndirectIndexed)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0x91;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x91;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;
    memory[0x130009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], 0x11);
}

TEST_F(CpuTest, TEST_STA_DirectIndirect)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0x92;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x92;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STA_DirectIndexedX)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x95;
    memory[GetPC() + 1] = 0xFE;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x000008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x000009], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x95;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x000008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x000009], 0x11);
}

TEST_F(CpuTest, TEST_STA_DirectIndirectLongIndexed)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0x97;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x130006], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130007], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x97;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;
    memory[0x130007] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x130006], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130007], 0x11);
}

TEST_F(CpuTest, TEST_STA_AbsoluteIndexedY)
{
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0x99;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.y = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x99;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], 0x11);
}

TEST_F(CpuTest, TEST_STA_AbsoluteIndexedX)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x9D;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x9D;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], 0x11);
}

TEST_F(CpuTest, TEST_STA_AbsoluteLongIndexedX)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x9F;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], Bytes::GetByte<1>(A_VALUE));

    ResetState();
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x9F;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[GetPC() + 3] = 0x12;
    memory[0x130009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x130008], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[0x130009], 0x11);
}

TEST_F(CpuTest, TEST_STX_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0x86;
    memory[GetPC() + 1] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x00FFFF], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[0x000000], Bytes::GetByte<1>(X_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x86;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[0x00FFFF], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[0x000000], 0x11);
}

TEST_F(CpuTest, TEST_STX_Absolute)
{
    memory[GetPC()] = 0x8E;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(X_VALUE));

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x8E;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STX_DirectIndexedY)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0x96;
    memory[GetPC() + 1] = 0xFE;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x000008], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[0x000009], Bytes::GetByte<1>(X_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x96;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0x000A);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[0x000008], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[0x000009], 0x11);
}

TEST_F(CpuTest, TEST_STY_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0x84;
    memory[GetPC() + 1] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x00FFFF], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[0x000000], Bytes::GetByte<1>(Y_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x84;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[0x00FFFF], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[0x000000], 0x11);
}

TEST_F(CpuTest, TEST_STY_Absolute)
{
    memory[GetPC()] = 0x8C;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[0x130000], Bytes::GetByte<1>(Y_VALUE));

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x8C;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[0x12FFFF], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STY_DirectIndexedX)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x94;
    memory[GetPC() + 1] = 0xFE;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x000008], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[0x000009], Bytes::GetByte<1>(Y_VALUE));

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x94;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[0x000008], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[0x000009], 0x11);
}

TEST_F(CpuTest, TEST_STZ_Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0x64;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0x11;
    memory[0x000000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x00FFFF], 0);
    ASSERT_EQ(memory[0x000000], 0);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x64;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0x11;
    memory[0x000000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x00FFFF], 0);
    ASSERT_EQ(memory[0x000000], 0x11);
}

TEST_F(CpuTest, TEST_STZ_DirectIndexedX)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x74;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0x11;
    memory[0x000009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x000008], 0);
    ASSERT_EQ(memory[0x000009], 0);

    ResetState();
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x74;
    memory[GetPC() + 1] = 0xFE;
    memory[0x000008] = 0x11;
    memory[0x000009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x000008], 0);
    ASSERT_EQ(memory[0x000009], 0x11);
}

TEST_F(CpuTest, TEST_STZ_Absolute)
{
    memory[GetPC()] = 0x9C;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0x11;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x12FFFF], 0);
    ASSERT_EQ(memory[0x130000], 0);

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x9C;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0xFF;
    memory[0x12FFFF] = 0x11;
    memory[0x130000] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x12FFFF], 0);
    ASSERT_EQ(memory[0x130000], 0x11);
}

TEST_F(CpuTest, TEST_STZ_AbsoluteIndexedX)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0x9E;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0x11;
    memory[0x130009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[0x130008], 0);
    ASSERT_EQ(memory[0x130009], 0);

    ResetState();
    cpu->reg.x = 0x000A;
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x9E;
    memory[GetPC() + 1] = 0xFE;
    memory[GetPC() + 2] = 0xFF;
    memory[0x130008] = 0x11;
    memory[0x130009] = 0x11;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0x000A);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[0x130008], 0);
    ASSERT_EQ(memory[0x130009], 0x11);
}

TEST_F(CpuTest, TEST_Push8Bit)
{
    cpu->reg.emulationMode = true;
    cpu->reg.sp = 0x0100;
    Push8Bit(0xAB);
    ASSERT_EQ(cpu->reg.sp, 0x01FF);
    ASSERT_EQ(memory[0x0100], 0xAB);
}

TEST_F(CpuTest, TEST_Pop8Bit)
{
    cpu->reg.emulationMode = true;
    memory[0x0100] = 0xAB;
    cpu->reg.sp = 0x01FF;
    uint8_t value = Pop8Bit();
    ASSERT_EQ(cpu->reg.sp, 0x0100);
    ASSERT_EQ(value, 0xAB);
}

TEST_F(CpuTest, TEST_PHA)
{
    memory[GetPC()] = 0x48;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<1>(A_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], Bytes::GetByte<0>(A_VALUE));

    ResetState();
    cpu->reg.flags.m = 1;
    memory[GetPC()] = 0x48;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 1);
    ASSERT_EQ(cpu->reg.p, 0x20);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<0>(A_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], 0);
}

TEST_F(CpuTest, TEST_PHX)
{
    memory[GetPC()] = 0xDA;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<1>(X_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], Bytes::GetByte<0>(X_VALUE));

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0xDA;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 1);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<0>(X_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], 0);
}

TEST_F(CpuTest, TEST_PHY)
{
    memory[GetPC()] = 0x5A;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<1>(Y_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], Bytes::GetByte<0>(Y_VALUE));

    ResetState();
    cpu->reg.flags.x = 1;
    memory[GetPC()] = 0x5A;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 1);
    ASSERT_EQ(cpu->reg.p, 0x10);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<0>(Y_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], 0);
}

TEST_F(CpuTest, TEST_PHB)
{
    memory[GetPC()] = 0x8B;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 1);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[SP_VALUE], DB_VALUE);
}

TEST_F(CpuTest, TEST_PHD)
{
    memory[GetPC()] = 0x0B;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[SP_VALUE], Bytes::GetByte<1>(D_VALUE));
    ASSERT_EQ(memory[SP_VALUE - 1], Bytes::GetByte<0>(D_VALUE));
}

TEST_F(CpuTest, TEST_PHK)
{
    memory[GetPC()] = 0x4B;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 1);
    ASSERT_EQ(cpu->reg.p, 0x00);
    ASSERT_EQ(memory[SP_VALUE], PB_VALUE);
}

TEST_F(CpuTest, TEST_PHP)
{
    cpu->reg.p = 0xFF;
    memory[GetPC()] = 0x08;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 1);
    ASSERT_EQ(memory[SP_VALUE], 0xFF);
}

TEST_F(CpuTest, TEST_PEA)
{
    memory[GetPC()] = 0xF4;
    memory[GetPC() + 1] = 0xCD;
    memory[GetPC() + 2] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(memory[SP_VALUE], 0xAB);
    ASSERT_EQ(memory[SP_VALUE - 1], 0xCD);
}

TEST_F(CpuTest, TEST_PEI)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xD4;
    memory[GetPC() + 1] = 0xFF;
    memory[0x00FFFF] = 0xCD;
    memory[0x000000] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xFF00);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(memory[SP_VALUE], 0xAB);
    ASSERT_EQ(memory[SP_VALUE - 1], 0xCD);
}

TEST_F(CpuTest, TEST_PER)
{
    memory[GetPC()] = 0x62;
    memory[GetPC() + 1] = 0x10;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(memory[SP_VALUE], 0x00);
    ASSERT_EQ(memory[SP_VALUE - 1], 0x12);

    ResetState();
    cpu->reg.pc = 0x10;
    memory[GetPC()] = 0x62;
    memory[GetPC() + 1] = 0xF0;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE - 2);
    ASSERT_EQ(memory[SP_VALUE], 0x00);
    ASSERT_EQ(memory[SP_VALUE - 1], 0x02);
}

TEST_F(CpuTest, TEST_PLA)
{
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0x68;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, 0xABCD);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FF);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.m = 1;
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0x68;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, Bytes::MaskByte<1>(A_VALUE) | 0xCD);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FE);
    ASSERT_EQ(cpu->reg.p, 0xA0);
}

TEST_F(CpuTest, TEST_PLX)
{
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0xFA;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, 0xABCD);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FF);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.x = 1;
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0xFA;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, Bytes::MaskByte<1>(X_VALUE) | 0xCD);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_PLY)
{
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0x7A;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, 0xABCD);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FF);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    cpu->reg.flags.x = 1;
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0x7A;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Bytes::MaskByte<1>(Y_VALUE) | 0xCD);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}

TEST_F(CpuTest, TEST_PLB)
{
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0xAB;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, 0xCD);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FE);
    ASSERT_EQ(cpu->reg.p, 0x80);
}

TEST_F(CpuTest, TEST_PLD)
{
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0x2B;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, 0xABCD);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FF);
    ASSERT_EQ(cpu->reg.p, 0x80);
}

TEST_F(CpuTest, TEST_PLP)
{
    cpu->reg.sp = 0x01FD;
    memory[GetPC()] = 0x28;
    memory[0x0001FE] = 0xCD;
    memory[0x0001FF] = 0xAB;
    cpu->ProcessOpCode();
    ASSERT_EQ(cpu->reg.a, A_VALUE);
    ASSERT_EQ(cpu->reg.x, X_VALUE);
    ASSERT_EQ(cpu->reg.y, Y_VALUE);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, 0x01FE);
    ASSERT_EQ(cpu->reg.p, 0xCD);
}