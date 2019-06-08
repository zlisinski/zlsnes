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

TEST_F(CpuTest, TEST_GetOpAbsolute) // a - Absolute
{
    uint16_t result;

    cpu->reg.db = 0x12;
    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = GetOpAbsolute();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndexedX) // a,x - Absolute,X
{
    uint16_t result;

    cpu->reg.db = 0x12;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = GetOpAbsoluteIndexedX();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndexedY) // a,y - Absolute,Y
{
    uint16_t result;

    cpu->reg.db = 0x12;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = GetOpAbsoluteIndexedY();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndirect) // (a) - (Absolute)
{
    uint32_t result;

    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0x56;
    memory[0x00FFFF] = 0x78;
    result = GetOpAbsoluteIndirect();
    ASSERT_EQ(result, 0x345678);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndexedIndirect) // (a,x) - (Absolute,X)
{
    uint32_t result;

    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;
    memory[0x340008] = 0x78;
    memory[0x340009] = 0x56;
    result = GetOpAbsoluteIndexedIndirect();
    ASSERT_EQ(result, 0x345678);
}

TEST_F(CpuTest, TEST_GetOpAccumulator) // A - Accumulator
{
    uint16_t result;

    result = GetOpAccumulator();
    ASSERT_EQ(result, A_VALUE);
}

TEST_F(CpuTest, TEST_GetOpDirect) // d - Direct
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xFF;
    memory[0x00FFFF] = 0x34;
    memory[0x000000] = 0x12;
    result = GetOpDirect();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndexedX) // d,x - Direct,X
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[0x000008] = 0x34;
    memory[0x000009] = 0x12;
    result = GetOpDirectIndexedX();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndexedY) // d,y - Direct,Y
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[0x000008] = 0x34;
    memory[0x000009] = 0x12;
    result = GetOpDirectIndexedY();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirect) // (d) - (Direct)
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = GetOpDirectIndirect();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirectLong) // [d] - [Direct]
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = GetOpDirectIndirectLong();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndexedIndirect) // (d,x) - (Direct,X)
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = GetOpDirectIndexedIndirect();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirectIndexed) // (d),y - (Direct),Y
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = GetOpDirectIndirectIndexed();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirectLongIndexed) // [d],y - [Direct],Y
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;
    memory[0x130006] = 0x34;
    memory[0x130007] = 0x12;
    result = GetOpDirectIndirectLongIndexed();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteLong) // al - Long
{
    uint16_t result;

    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0x12;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = GetOpAbsoluteLong();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteLongIndexedX) // al,x - Long,X
{
    uint16_t result;

    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0x12;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = GetOpAbsoluteLongIndexedX();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpStackRelative) // d,s - Stack,S
{
    uint16_t result;

    cpu->reg.sp = 0xFF10;
    memory[GetPC()] = 0xFA;
    memory[0x00000A] = 0x34;
    memory[0x00000B] = 0x12;
    result = GetOpStackRelative();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpStackRelativeIndirectIndexed) // (d,s),y - (Stack,S),Y
{
    uint16_t result;

    cpu->reg.sp = 0xFF10;
    cpu->reg.y = 0x0050;
    memory[GetPC()] = 0xFA;
    memory[0x00000A] = 0xF0;
    memory[0x00000B] = 0xFF;
    memory[0x130040] = 0x34;
    memory[0x130041] = 0x12;
    result = GetOpStackRelativeIndirectIndexed();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_LoadRegister)
{
    uint16_t dest;

    ResetState();
    dest = 0;
    LoadRegister(&dest, 0x1234, true);
    ASSERT_EQ(dest, 0x1234);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    dest = 0xFFFF;
    LoadRegister(&dest, 0, true);
    ASSERT_EQ(dest, 0);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    dest = 0;
    LoadRegister(&dest, 0xFFFF, true);
    ASSERT_EQ(dest, 0xFFFF);
    ASSERT_EQ(cpu->reg.flags.n, 1);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x80);

    ResetState();
    dest = 0x1234;
    LoadRegister(&dest, 0, false);
    ASSERT_EQ(dest, 0x1200);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 1);
    ASSERT_EQ(cpu->reg.p, 0x02);

    ResetState();
    dest = 0x1234;
    LoadRegister(&dest, 0x5678, false);
    ASSERT_EQ(dest, 0x1278);
    ASSERT_EQ(cpu->reg.flags.n, 0);
    ASSERT_EQ(cpu->reg.flags.z, 0);
    ASSERT_EQ(cpu->reg.p, 0x00);

    ResetState();
    dest = 0x1234;
    LoadRegister(&dest, 0x80, false);
    ASSERT_EQ(dest, 0x1280);
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
    ASSERT_EQ(cpu->reg.x, X_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.y, Y_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.x, X_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.a, A_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.y, Y_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.a, A_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.x, X_VALUE & 0xFF00);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.a, (A_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.x, (X_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.x, (X_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.x, (X_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.x, (X_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.x, (X_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.y, (Y_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.y, (Y_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.y, (Y_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.y, (Y_VALUE & 0xFF00) | 0xEF);
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
    ASSERT_EQ(cpu->reg.y, (Y_VALUE & 0xFF00) | 0xEF);
    ASSERT_EQ(cpu->reg.d, D_VALUE);
    ASSERT_EQ(cpu->reg.db, DB_VALUE);
    ASSERT_EQ(cpu->reg.pb, PB_VALUE);
    ASSERT_EQ(cpu->reg.sp, SP_VALUE);
    ASSERT_EQ(cpu->reg.p, 0x90);
}