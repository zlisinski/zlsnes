#include "AddressModeTest.h"
#include "../Timer.h"

const uint16_t A_VALUE = 0x1234;
const uint16_t X_VALUE = 0x5678;
const uint16_t Y_VALUE = 0x9ABC;
const uint16_t D_VALUE = 0xDEF0;
const uint8_t P_VALUE = 0x00;
const uint8_t DB_VALUE = 0x12;
const uint8_t PB_VALUE = 0x34;
const uint16_t SP_VALUE = 0xFFFF;


AddressModeTest::AddressModeTest()
{
    memory_ = new Memory();
    timer = new Timer();
    cpu = new Cpu(memory_, timer);
    memory_->SetTimer(timer);
}

AddressModeTest::~AddressModeTest()
{
    delete cpu;
    delete timer;
    delete memory_;
}

void AddressModeTest::SetUp()
{
    ResetState();
}

void AddressModeTest::TearDown()
{

}

void AddressModeTest::ResetState()
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


TEST_F(AddressModeTest, TEST_AddressModeAbsolute) // a - Absolute
{
    cpu->reg.db = 0x12;
    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;

    AddressModeAbsolute mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x12FFFF);

    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;

    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeAbsoluteIndexedX) // a,x - Absolute,X
{
    cpu->reg.db = 0x12;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;

    AddressModeAbsoluteIndexedX mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x130008);

    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeAbsoluteIndexedY) // a,y - Absolute,Y
{
    cpu->reg.db = 0x12;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;

    AddressModeAbsoluteIndexedY mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x130008);

    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeAbsoluteIndirect) // (a) - (Absolute)
{
    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0x56;
    memory[0x00FFFF] = 0x78;
    AddressModeAbsoluteIndirect mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x005678);
}

TEST_F(AddressModeTest, TEST_AddressModeAddressModeAbsoluteIndirectLong) // [a] - [Absolute]
{
    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;
    memory[0x000000] = 0x56;
    memory[0x000001] = 0x98;
    memory[0x00FFFF] = 0x78;
    AddressModeAbsoluteIndirectLong mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x985678);
}

TEST_F(AddressModeTest, TEST_AddressModeAbsoluteIndexedIndirect) // (a,x) - (Absolute,X)
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;
    memory[0x340008] = 0x78;
    memory[0x340009] = 0x56;
    AddressModeAbsoluteIndexedIndirect mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x345678);
}

TEST_F(AddressModeTest, TEST_AddressModeDirect) // d - Direct
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xFF;

    AddressModeDirect mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x00FFFF);

    memory[0x00FFFF] = 0x34;
    memory[0x000000] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndexedX) // d,x - Direct,X
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;

    AddressModeDirectIndexedX mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x000008);

    memory[0x000008] = 0x34;
    memory[0x000009] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndexedY) // d,y - Direct,Y
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFE;

    AddressModeDirectIndexedY mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x000008);

    memory[0x000008] = 0x34;
    memory[0x000009] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndirect) // (d) - (Direct)
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0xFF;

    AddressModeDirectIndirect mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x12FFFF);

    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndirectLong) // [d] - [Direct]
{
    cpu->reg.d = 0xFF00;
    memory[GetPC()] = 0xFE;
    memory[0x00FFFE] = 0xFF;
    memory[0x00FFFF] = 0xFF;
    memory[0x000000] = 0x12;

    AddressModeDirectIndirectLong mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x12FFFF);

    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndexedIndirect) // (d,x) - (Direct,X)
{
    cpu->reg.d = 0xFF00;
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[0x000008] = 0xFF;
    memory[0x000009] = 0xFF;

    AddressModeDirectIndexedIndirect mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x12FFFF);

    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndirectIndexed) // (d),y - (Direct),Y
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFF;
    memory[0x000000] = 0xFF;
    memory[0x00FFFF] = 0xFE;

    AddressModeDirectIndirectIndexed mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x130008);

    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectIndirectLongIndexed) // [d],y - [Direct],Y
{
    cpu->reg.d = 0xFF00;
    cpu->reg.y = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[0x000000] = 0x12;
    memory[0x00FFFE] = 0xFC;
    memory[0x00FFFF] = 0xFF;

    AddressModeDirectIndirectLongIndexed mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x130006);

    memory[0x130006] = 0x34;
    memory[0x130007] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeDirectImmediate) // Immediate
{
    memory[GetPC()] = 0xCD;
    memory[GetPC() + 1] = 0xAB;

    AddressModeImmediate mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x00);

    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0xABCD);
}

TEST_F(AddressModeTest, TEST_AddressModeAbsoluteLong) // al - Long
{
    memory[GetPC()] = 0xFF;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0x12;

    AddressModeAbsoluteLong mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x12FFFF);

    memory[0x12FFFF] = 0x34;
    memory[0x130000] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeAbsoluteLongIndexedX) // al,x - Long,X
{
    cpu->reg.x = 0x000A;
    memory[GetPC()] = 0xFE;
    memory[GetPC() + 1] = 0xFF;
    memory[GetPC() + 2] = 0x12;

    AddressModeAbsoluteLongIndexedX mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x130008);

    memory[0x130008] = 0x34;
    memory[0x130009] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeAccumulator) // A - Accumulator
{
    AddressModeAccumulator mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x00);

    ASSERT_EQ(mode.Read8Bit(), A_VALUE & 0xFF);
    ASSERT_EQ(mode.Read16Bit(), A_VALUE);

    mode.Write16Bit(0x5678);
    ASSERT_EQ(cpu->reg.a, 0x5678);
    mode.Write8Bit(0xEF);
    ASSERT_EQ(cpu->reg.a, 0x56EF);
}

TEST_F(AddressModeTest, TEST_AddressModeStackRelative) // d,s - Stack,S
{
    cpu->reg.sp = 0xFF10;
    memory[GetPC()] = 0xFA;

    AddressModeStackRelative mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x00000A);

    memory[0x00000A] = 0x34;
    memory[0x00000B] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}

TEST_F(AddressModeTest, TEST_AddressModeStackRelativeIndirectIndexed) // (d,s),y - (Stack,S),Y
{
    cpu->reg.sp = 0xFF10;
    cpu->reg.y = 0x0050;
    memory[GetPC()] = 0xFA;
    memory[0x00000A] = 0xF0;
    memory[0x00000B] = 0xFF;

    AddressModeStackRelativeIndirectIndexed mode(cpu, memory_);
    mode.LoadAddress();
    Address addr = mode.GetAddress();
    ASSERT_EQ(addr.ToUint(), 0x130040);

    memory[0x130040] = 0x34;
    memory[0x130041] = 0x12;
    uint16_t result = mode.Read16Bit();
    ASSERT_EQ(result, 0x1234);
}