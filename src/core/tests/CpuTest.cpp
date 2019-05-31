#include <stdint.h>

#include "CpuTest.h"
#include "../Cpu.h"
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

void PrintTo(uint32_t, std::ostream *os)
{
    *os << "blarg";
}


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

    memory_->ClearMemory();
    memory = memory_->GetBytePtr(0);
}

///////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTest, TEST_GetOpAbsolute) // a - Absolute
{
    uint16_t result;

    cpu->reg.db = 0x12;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFF;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = cpu->GetOpAbsolute();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndexedX) // a,x - Absolute,X
{
    uint16_t result;

    cpu->reg.db = 0x12;
    cpu->reg.x = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = cpu->GetOpAbsoluteIndexedX();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndexedY) // a,y - Absolute,Y
{
    uint16_t result;

    cpu->reg.db = 0x12;
    cpu->reg.y = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = cpu->GetOpAbsoluteIndexedY();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndirect) // (a) - (Absolute)
{
    uint32_t result;

    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFF;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[0x000000] = 0x56;
    memory[0x00FFFF] = 0x78;
    result = cpu->GetOpAbsoluteIndirect();
    ASSERT_EQ(result, 0x345678);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteIndexedIndirect) // (a,x) - (Absolute,X)
{
    uint32_t result;

    cpu->reg.x = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[0x340008] = 0x78;
    memory[0x340009] = 0x56;
    result = cpu->GetOpAbsoluteIndexedIndirect();
    ASSERT_EQ(result, 0x345678);
}

TEST_F(CpuTest, TEST_GetOpAccumulator) // A - Accumulator
{
    uint16_t result;

    result = cpu->GetOpAccumulator();
    ASSERT_EQ(result, A_VALUE);
}

TEST_F(CpuTest, TEST_GetOpDirect) // d - Direct
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFF;
    memory[0x00FFFF] = 0x34;
    memory[0x000000] = 0x12;
    result = cpu->GetOpDirect();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndexedX) // d,x - Direct,X
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[0x000008] = 0x34;
    memory[0x000009] = 0x12;
    result = cpu->GetOpDirectIndexedX();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndexedY) // d,y - Direct,Y
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[0x000008] = 0x34;
    memory[0x000009] = 0x12;
    result = cpu->GetOpDirectIndexedY();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirect) // (d) - (Direct)
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = cpu->GetOpDirectIndirect();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirectLong) // [d] - [Direct]
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = cpu->GetOpDirectIndirectLong();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndexedIndirect) // (d,x) - (Direct,X)
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = cpu->GetOpDirectIndexedIndirect();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirectIndexed) // (d),y - (Direct),Y
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = cpu->GetOpDirectIndirectIndexed();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpDirectIndirectLongIndexed) // [d],y - [Direct],Y
{
    uint16_t result;

    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;
    memory[0x130006] = 0x34;
    memory[0x130007] = 0x12;
    result = cpu->GetOpDirectIndirectLongIndexed();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteLong) // al - Long
{
    uint16_t result;

    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFF;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 2)] = 0x12;
    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    result = cpu->GetOpAbsoluteLong();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(CpuTest, TEST_GetOpAbsoluteLongIndexedX) // al,x - Long,X
{
    uint16_t result;

    cpu->reg.x = 0x000A;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc)] = 0xFE;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 1)] = 0xFF;
    memory[Make24Bit(cpu->reg.pb, cpu->reg.pc + 2)] = 0x12;
    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    result = cpu->GetOpAbsoluteLongIndexedX();
    ASSERT_EQ(result, 0x1234);
}