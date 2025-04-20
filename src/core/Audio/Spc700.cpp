#include "AddressMode.h"
#include "Spc700.h"
#include "Memory.h"
#include "Timer.h"


namespace Audio
{


// Use this for opcodes that don't have an AddressMode or data.
#define LogInst(name) do {LogSpc700("%02X: %s", opcode, (name));} while (0)

// Opcodes with data but no AddressMode
#define LogInst1(name, param) do {LogSpc700("%02X %02X: %s", opcode, (param), (name));} while (0)
#define LogInst2(name, param1, param2) do {LogSpc700("%02X %02X %02X: %s", opcode, (param1), (param2), (name));} while (0)

// Opcodes with an AddressMode
#define LogInstM(name, addrmode) do {(addrmode)->Log(name);} while (0)


Spc700::Spc700(Memory *memory, Timer *timer) :
    memory(memory),
    timer(timer)
{
    addressModes[0x04] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x05] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x06] = std::make_unique<AddressModeIndirectX>(this, memory);
    addressModes[0x07] = std::make_unique<AddressModeIndirectIndexedX>(this, memory);
    addressModes[0x08] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x0B] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x0C] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0D] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x14] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x15] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x16] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory);
    addressModes[0x17] = std::make_unique<AddressModeIndirectIndexedY>(this, memory);
    addressModes[0x1A] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x1B] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x1C] = std::make_unique<AddressModeAccumulator>(this, memory);

    addressModesMovXY[0x09] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModesMovXY[0x0B] = std::make_unique<AddressModeDirect>(this, memory);
    addressModesMovXY[0x0C] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModesMovXY[0x0D] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModesMovXY[0x18] = std::make_unique<AddressModeDirect>(this, memory);
    addressModesMovXY[0x19] = std::make_unique<AddressModeDirectIndexedY>(this, memory);
    addressModesMovXY[0x1B] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
}


Spc700::~Spc700()
{

}


uint8_t Spc700::ReadPC8Bit()
{
    uint8_t byte = memory->Read8Bit(reg.pc);
    reg.pc++;

    return byte;
}


uint16_t Spc700::ReadPC16Bit()
{
    uint8_t low = memory->Read8Bit(reg.pc);
    reg.pc++;

    uint8_t high = memory->Read8Bit(reg.pc);
    reg.pc++;

    return (high << 8) | low;
}


inline uint8_t Spc700::Add8Bit(uint8_t x, uint8_t y)
{
    uint16_t result16 = x + y + reg.flags.c;
    uint8_t result8 = static_cast<uint8_t>(result16);

    reg.flags.v = (((x ^ result16) & ~(x ^ y)) & 0x80) != 0;
    reg.flags.h = ((x & 0x0F) + (y & 0x0F) + reg.flags.c) > 0x0F;
    reg.flags.c = result16 > 0xFF;
    SetNFlag(result8);
    SetZFlag(result8);

    return result8;
}


inline uint8_t Spc700::Sub8Bit(uint8_t x, uint8_t y)
{
    y = ~y;
    uint16_t result16 = x + y + reg.flags.c;
    uint8_t result8 = static_cast<uint8_t>(result16);

    reg.flags.v = (((x ^ result16) & ~(x ^ y)) & 0x80) != 0;
    reg.flags.h = ((x & 0x0F) + (y & 0x0F) + reg.flags.c) > 0x0F;
    reg.flags.c = static_cast<int8_t>(result16 >> 8) > 0;
    SetNFlag(result8);
    SetZFlag(result8);

    return result8;
}


inline void Spc700::Compare(uint8_t x, uint8_t y)
{
    uint8_t result = x - y;
    reg.flags.c = x >= y;
    SetNFlag(result);
    SetZFlag(result);
}


inline void Spc700::Push(uint8_t value)
{
    memory->Write8Bit(Bytes::Make16Bit(0x01, reg.sp), value);
    reg.sp--;
}


inline uint8_t Spc700::Pop()
{
    reg.sp++;
    uint8_t value = memory->Read8Bit(Bytes::Make16Bit(0x01, reg.sp));
    return value;
}


void Spc700::Step(int clocksToRun)
{
    clocksAhead -= clocksToRun;

    if (clocksAhead <= 0)
    {
        timer->ResetCounter();

        do
        {
            ProcessOpCode();
            clocksAhead += timer->GetCounter();
        }
        while (clocksAhead < 0);
    }
}


void Spc700::ProcessOpCode()
{
    opcode = ReadPC8Bit();

    switch (opcode)
    {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// MOV memory to register opcodes                                                                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // MOV A, n
        case 0xE4: // MOV A, Direct
        case 0xE5: // MOV A, !Absolute
        case 0xE6: // MOV A, IndirectX
        case 0xE7: // MOV A, IndirectIndexedX
        case 0xE8: // MOV A, Immediate
        case 0xF4: // MOV A, DirectIndexedX
        case 0xF5: // MOV A, !AbsoluteIndexedX
        case 0xF6: // MOV A, !AbsoluteIndexedY
        case 0xF7: // MOV A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("MOV A, ", mode);
            LoadRegister(reg.a, mode->Read8Bit());
            break;
        }
        case 0xBF: // MOV A, (X)+
        {
            AddressModeIndirectX mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOV A, (X)+", &mode);
            LoadRegister(reg.a, mode.Read8Bit());
            reg.x++;
            break;
        }

        // MOV X, n
        case 0xCD: // MOV X, Immediate
        case 0xE9: // MOV X, Absolute
        case 0xF8: // MOV X, Direct
        case 0xF9: // MOV X, Direct,Y
        {
            AddressModePtr &mode = addressModesMovXY[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("MOV X, ", mode);
            LoadRegister(reg.x, mode->Read8Bit());
            break;
        }

        // MOV Y, n
        case 0x8D: // MOV Y, Immediate
        case 0xEB: // MOV Y, Direct
        case 0xEC: // MOV Y, Absolute
        case 0xFB: // MOV Y, Direct,X
        {
            AddressModePtr &mode = addressModesMovXY[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("MOV Y, ", mode);
            LoadRegister(reg.y, mode->Read8Bit());
            break;
        }

        case 0xBA: // MOVW YA, Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOVW YA,", &mode);
            reg.ya = mode.Read16Bit();
            SetNFlag(reg.ya);
            SetZFlag(reg.ya);
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// MOV register to memory opcodes                                                                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // MOV n, A
        case 0xC4: // MOV Direct, A
        case 0xC5: // MOV !Absolute, A
        case 0xC6: // MOV IndirectX, A
        case 0xC7: // MOV IndirectIndexedX, A
        case 0xD4: // MOV DirectIndexedX, A
        case 0xD5: // MOV !AbsoluteIndexedX, A
        case 0xD6: // MOV !AbsoluteIndexedY, A
        case 0xD7: // MOV IndirectIndexedY, A
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("MOV n, A", mode);
            mode->Write8Bit(reg.a);
            break;
        }
        case 0xAF: // MOV (X)+, A
        {
            AddressModeIndirectX mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOV (X)+, A", &mode);
            mode.Write8Bit(reg.a);
            reg.x++;
            break;
        }

        // MOV n, X
        case 0xC9: // MOV Absolute, X
        case 0xD8: // MOV Direct, X
        case 0xD9: // MOV Direct,Y, X
        {
            AddressModePtr &mode = addressModesMovXY[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("MOV n, X", mode);
            mode->Write8Bit(reg.x);
            break;
        }

        // MOV n, Y
        case 0xCB: // MOV Direct, Y
        case 0xCC: // MOV Absolute, Y
        case 0xDB: // MOV Direct,X, Y
        {
            AddressModePtr &mode = addressModesMovXY[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("MOV n, Y", mode);
            mode->Write8Bit(reg.y);
            break;
        }

        case 0xDA: // MOVW Direct, YA
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOVW dp, YA", &mode);
            mode.Write16Bit(reg.ya);
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// MOV register to register opcodes                                                                                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x7D: //MOV A, X
        {
            LogInst("MOV A, X");
            LoadRegister(reg.a, reg.x);
            break;
        }

        case 0xDD: //MOV A, Y
        {
            LogInst("MOV A, Y");
            LoadRegister(reg.a, reg.y);
            break;
        }

        case 0x5D: //MOV X, A
        {
            LogInst("MOV X, A");
            LoadRegister(reg.x, reg.a);
            break;
        }

        case 0xFD: //MOV Y, A
        {
            LogInst("MOV Y, A");
            LoadRegister(reg.y, reg.a);
            break;
        }

        case 0x9D: //MOV X, SP
        {
            LogInst("MOV X, SP");
            LoadRegister(reg.x, reg.sp);
            break;
        }

        case 0xBD: //MOV SP, X
        {
            LogInst("MOV SP, X");
            reg.sp = reg.x; // No flags set.
            break;
        }

        case 0xFA: //MOV Direct, Direct
        {
            LogInst("MOV dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            dest.Write8Bit(src.Read8Bit());
            break;
        }

        case 0x8F: //MOV Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOV dp, immediate", &mode);
            mode.Write8Bit(byte);
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Arithmetic opcodes                                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x84: // ADC A, Direct
        case 0x85: // ADC A, Absolute
        case 0x86: // ADC A, IndirectX
        case 0x87: // ADC A, IndirectIndexedX
        case 0x88: // ADC A, Immediate
        case 0x94: // ADC A, DirectIndexedX
        case 0x95: // ADC A, AbsoluteIndexedX
        case 0x96: // ADC A, AbsoluteIndexedY
        case 0x97: // ADC A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("ADC A, ", mode);
            reg.a = Add8Bit(reg.a, mode->Read8Bit());
            break;
        }

        case 0x99: // ADC (X), (Y)
        {
            LogInst("ADC (X),(Y)");
            AddressModeIndirectX dest(this, memory);
            dest.LoadAddress();
            AddressModeIndirectY src(this, memory);
            src.LoadAddress();
            dest.Write8Bit(Add8Bit(dest.Read8Bit(), src.Read8Bit()));
            break;
        }

        case 0x89: // ADC Direct, Direct
        {
            LogInst("ADC dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            dest.Write8Bit(Add8Bit(dest.Read8Bit(), src.Read8Bit()));
            break;
        }

        case 0x98: // ADC Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("ADC dp, immediate", &mode);
            mode.Write8Bit(Add8Bit(mode.Read8Bit(), byte));
            break;
        }

        case 0x7A: // ADDW YA, Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("ADDW YA,", &mode);

            uint16_t operand = mode.Read16Bit();
            uint32_t result32 = reg.ya + operand;
            uint16_t result16 = static_cast<uint16_t>(result32);

            reg.flags.v = (((reg.ya ^ result32) & ~(reg.ya ^ operand)) & 0x8000) != 0;
            reg.flags.h = ((reg.ya & 0xFFF) + (operand & 0xFFF)) > 0xFFF;
            reg.flags.c = result32 > 0xFFFF;
            SetNFlag(result16);
            SetZFlag(result16);
            reg.ya = result16;
            break;
        }

        case 0xA4: // SBC A, Direct
        case 0xA5: // SBC A, Absolute
        case 0xA6: // SBC A, IndirectX
        case 0xA7: // SBC A, IndirectIndexedX
        case 0xA8: // SBC A, Immediate
        case 0xB4: // SBC A, DirectIndexedX
        case 0xB5: // SBC A, AbsoluteIndexedX
        case 0xB6: // SBC A, AbsoluteIndexedY
        case 0xB7: // SBC A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("SBC A, ", mode);
            reg.a = Sub8Bit(reg.a, mode->Read8Bit());
            break;
        }

        case 0xB9: // SBC (X), (Y)
        {
            LogInst("SBC (X),(Y)");
            AddressModeIndirectX dest(this, memory);
            dest.LoadAddress();
            AddressModeIndirectY src(this, memory);
            src.LoadAddress();
            dest.Write8Bit(Sub8Bit(dest.Read8Bit(), src.Read8Bit()));
            break;
        }

        case 0xA9: // SBC Direct, Direct
        {
            LogInst("SBC dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            dest.Write8Bit(Sub8Bit(dest.Read8Bit(), src.Read8Bit()));
            break;
        }

        case 0xB8: // SBC Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("SBC dp, immediate", &mode);
            mode.Write8Bit(Sub8Bit(mode.Read8Bit(), byte));
            break;
        }

        case 0x9A: // SUBW YA, Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("SUBW YA,", &mode);

            uint16_t operand = ~mode.Read16Bit() + 1;
            uint32_t result32 = reg.ya + operand;
            uint16_t result16 = static_cast<uint16_t>(result32);

            reg.flags.v = (((reg.ya ^ result32) & ~(reg.ya ^ operand)) & 0x8000) != 0;
            reg.flags.h = ((reg.ya & 0xFFF) + (operand & 0xFFF)) > 0xFFF;
            reg.flags.c = static_cast<int16_t>(result32 >> 16) > 0;
            SetNFlag(result16);
            SetZFlag(result16);
            reg.ya = result16;
            break;
        }

        case 0x64: // CMP A, Direct
        case 0x65: // CMP A, Absolute
        case 0x66: // CMP A, IndirectX
        case 0x67: // CMP A, IndirectIndexedX
        case 0x68: // CMP A, Immediate
        case 0x74: // CMP A, DirectIndexedX
        case 0x75: // CMP A, AbsoluteIndexedX
        case 0x76: // CMP A, AbsoluteIndexedY
        case 0x77: // CMP A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("CMP A, ", mode);
            Compare(reg.a, mode->Read8Bit());
            break;
        }

        case 0x79: // CMP (X), (Y)
        {
            LogInst("CMP (X),(Y)");
            AddressModeIndirectX dest(this, memory);
            dest.LoadAddress();
            AddressModeIndirectY src(this, memory);
            src.LoadAddress();
            Compare(dest.Read8Bit(), src.Read8Bit());
            break;
        }

        case 0x69: // CMP Direct, Direct
        {
            LogInst("CMP dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            Compare(dest.Read8Bit(), src.Read8Bit());
            break;
        }

        case 0x78: // CMP Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP dp, immediate", &mode);
            Compare(mode.Read8Bit(), byte);
            break;
        }

        case 0xC8: // CMP X, Immediate
        {
            AddressModeImmediate mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP X, ", &mode);
            Compare(reg.x, mode.Read8Bit());
            break;
        }

        case 0x3E: // CMP X, Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP X, ", &mode);
            Compare(reg.x, mode.Read8Bit());
            break;
        }

        case 0x1E: // CMP X, Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP X, ", &mode);
            Compare(reg.x, mode.Read8Bit());
            break;
        }

        case 0xAD: // CMP Y, Immediate
        {
            AddressModeImmediate mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP Y, ", &mode);
            Compare(reg.y, mode.Read8Bit());
            break;
        }

        case 0x7E: // CMP Y, Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP Y, ", &mode);
            Compare(reg.y, mode.Read8Bit());
            break;
        }

        case 0x5E: // CMP Y, Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMP Y, ", &mode);
            Compare(reg.y, mode.Read8Bit());
            break;
        }

        case 0x5A: // CMPW YA, Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("CMPW YA,", &mode);

            uint16_t operand = mode.Read16Bit();
            uint16_t result = reg.ya - operand;
            reg.flags.c = reg.ya >= operand;
            SetNFlag(result);
            SetZFlag(result);
            break;
        }

        case 0xCF: // MUL YA
        {
            LogInst("MUL YA");
            reg.ya = reg.y * reg.a;
            SetNFlag(reg.y);
            SetZFlag(reg.y);
            break;
        }

        case 0x9E: // DIV YA, X
        {
            LogInst("DIV YA, X");

            uint32_t ya = reg.ya;
            uint32_t x = reg.x << 9;
            bool carry;

            for (int i = 0; i < 9; i++)
            {
                carry = ya >> 16;
                ya = ((ya << 1) | carry) & 0x01FFFF;
                ya ^= (ya >= x);
                if (ya & 0x01)
                    ya = (ya - x) & 0x01FFFF;
            }

            reg.flags.h = (reg.x & 0x0F) <= (reg.y & 0x0F);
            reg.flags.v = (ya & 0x0100) != 0;

            reg.a = ya;
            reg.y = ya >> 9;

            SetNFlag(reg.a);
            SetZFlag(reg.a);

            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Logical opcodes                                                                                                   //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x24: // AND A, Direct
        case 0x25: // AND A, Absolute
        case 0x26: // AND A, IndirectX
        case 0x27: // AND A, IndirectIndexedX
        case 0x28: // AND A, Immediate
        case 0x34: // AND A, DirectIndexedX
        case 0x35: // AND A, AbsoluteIndexedX
        case 0x36: // AND A, AbsoluteIndexedY
        case 0x37: // AND A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("AND A, ", mode);
            uint8_t result = reg.a & mode->Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            reg.a = result;
            break;
        }

        case 0x39: // AND (X), (Y)
        {
            LogInst("AND (X),(Y)");
            AddressModeIndirectX dest(this, memory);
            dest.LoadAddress();
            AddressModeIndirectY src(this, memory);
            src.LoadAddress();
            uint8_t result = dest.Read8Bit() & src.Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            dest.Write8Bit(result);
            break;
        }

        case 0x29: // AND Direct, Direct
        {
            LogInst("AND dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            uint8_t result = dest.Read8Bit() & src.Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            dest.Write8Bit(result);
            break;
        }

        case 0x38: // AND Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("AND dp, immediate", &mode);
            uint8_t result = mode.Read8Bit() & byte;
            SetNFlag(result);
            SetZFlag(result);
            mode.Write8Bit(result);
            break;
        }

        case 0x04: // OR A, Direct
        case 0x05: // OR A, Absolute
        case 0x06: // OR A, IndirectX
        case 0x07: // OR A, IndirectIndexedX
        case 0x08: // OR A, Immediate
        case 0x14: // OR A, DirectIndexedX
        case 0x15: // OR A, AbsoluteIndexedX
        case 0x16: // OR A, AbsoluteIndexedY
        case 0x17: // OR A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("OR A, ", mode);
            uint8_t result = reg.a | mode->Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            reg.a = result;
            break;
        }

        case 0x19: // OR (X), (Y)
        {
            LogInst("OR (X),(Y)");
            AddressModeIndirectX dest(this, memory);
            dest.LoadAddress();
            AddressModeIndirectY src(this, memory);
            src.LoadAddress();
            uint8_t result = dest.Read8Bit() | src.Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            dest.Write8Bit(result);
            break;
        }

        case 0x09: // OR Direct, Direct
        {
            LogInst("OR dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            uint8_t result = dest.Read8Bit() | src.Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            dest.Write8Bit(result);
            break;
        }

        case 0x18: // OR Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("OR dp, immediate", &mode);
            uint8_t result = mode.Read8Bit() | byte;
            SetNFlag(result);
            SetZFlag(result);
            mode.Write8Bit(result);
            break;
        }

        case 0x44: // EOR A, Direct
        case 0x45: // EOR A, Absolute
        case 0x46: // EOR A, IndirectX
        case 0x47: // EOR A, IndirectIndexedX
        case 0x48: // EOR A, Immediate
        case 0x54: // EOR A, DirectIndexedX
        case 0x55: // EOR A, AbsoluteIndexedX
        case 0x56: // EOR A, AbsoluteIndexedY
        case 0x57: // EOR A, IndirectIndexedY
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("EOR A, ", mode);
            uint8_t result = reg.a ^ mode->Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            reg.a = result;
            break;
        }

        case 0x59: // EOR (X), (Y)
        {
            LogInst("EOR (X),(Y)");
            AddressModeIndirectX dest(this, memory);
            dest.LoadAddress();
            AddressModeIndirectY src(this, memory);
            src.LoadAddress();
            uint8_t result = dest.Read8Bit() ^ src.Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            dest.Write8Bit(result);
            break;
        }

        case 0x49: // EOR Direct, Direct
        {
            LogInst("EOR dp, dp");
            AddressModeDirect src(this, memory);
            src.LoadAddress();
            AddressModeDirect dest(this, memory);
            dest.LoadAddress();
            uint8_t result = dest.Read8Bit() ^ src.Read8Bit();
            SetNFlag(result);
            SetZFlag(result);
            dest.Write8Bit(result);
            break;
        }

        case 0x58: // EOR Direct, Immediate
        {
            uint8_t byte = ReadPC8Bit();
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("EOR dp, immediate", &mode);
            uint8_t result = mode.Read8Bit() ^ byte;
            SetNFlag(result);
            SetZFlag(result);
            mode.Write8Bit(result);
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Bit opcodes                                                                                                       //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x02: // SET1 Direct, 0
        case 0x22: // SET1 Direct, 1
        case 0x42: // SET1 Direct, 2
        case 0x62: // SET1 Direct, 3
        case 0x82: // SET1 Direct, 4
        case 0xA2: // SET1 Direct, 5
        case 0xC2: // SET1 Direct, 6
        case 0xE2: // SET1 Direct, 7
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("SET1", &mode);

            uint8_t bit = opcode >> 5;
            uint8_t value = mode.Read8Bit();
            value |= 1 << bit;
            mode.Write8Bit(value);

            break;
        }

        case 0x12: // CLR1 Direct, 0
        case 0x32: // CLR1 Direct, 1
        case 0x52: // CLR1 Direct, 2
        case 0x72: // CLR1 Direct, 3
        case 0x92: // CLR1 Direct, 4
        case 0xB2: // CLR1 Direct, 5
        case 0xD2: // CLR1 Direct, 6
        case 0xF2: // CLR1 Direct, 7
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("CLR1", &mode);

            uint8_t bit = opcode >> 5;
            uint8_t value = mode.Read8Bit();
            value &= ~(1 << bit);
            mode.Write8Bit(value);

            break;
        }

        case 0x0E: // TSET1 Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("TSET1", &mode);

            uint8_t value = mode.Read8Bit();
            mode.Write8Bit(value | reg.a);
            value = reg.a - value;
            SetNFlag(value);
            SetZFlag(value);

            break;
        }

        case 0x4E: // TCLR1 Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("TCLR1", &mode);

            uint8_t value = mode.Read8Bit();
            mode.Write8Bit(value & ~reg.a);
            value = reg.a - value;
            SetNFlag(value);
            SetZFlag(value);

            break;
        }

        case 0x4A: // AND1 Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("AND1", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            reg.flags.c &= (memory->Read8Bit(addr) >> bit) & 0x01;

            break;
        }

        case 0x6A: // AND1 not Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("AND1 !", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            reg.flags.c &= ~(memory->Read8Bit(addr) >> bit) & 0x01;

            break;
        }

        case 0x0A: // OR1 Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("OR1", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            reg.flags.c |= (memory->Read8Bit(addr) >> bit) & 0x01;

            break;
        }

        case 0x2A: // OR1 not Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("OR1 !", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            reg.flags.c |= ~(memory->Read8Bit(addr) >> bit) & 0x01;

            break;
        }

        case 0x8A: // EOR1 Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("EOR1", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            reg.flags.c ^= (memory->Read8Bit(addr) >> bit) & 0x01;

            break;
        }

        case 0xEA: // NOT1 Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("NOT1", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            uint8_t value = memory->Read8Bit(addr);
            value ^= (1 << bit);
            memory->Write8Bit(addr, value);

            break;
        }

        case 0xAA: // MOV1 C, Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOV1 C, Abs", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            reg.flags.c = (memory->Read8Bit(addr) >> bit) & 0x01;

            break;
        }

        case 0xCA: // MOV1 Absolute, C
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("MOV1 Abs, C", &mode);

            uint16_t addr = mode.GetAddress();
            uint8_t bit = addr >> 13;
            addr &= 0x1FFF;
            uint8_t value = memory->Read8Bit(addr) & ~(1 << bit);
            value |= reg.flags.c << bit;
            memory->Write8Bit(addr, value);

            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Increment/Decrement opcodes                                                                                       //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0xBC: // INC A
        case 0xAB: // INC Direct
        case 0xBB: // INC DirectIndexedX
        case 0xAC: // INC Absolute
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("INC", mode);
            uint8_t result = mode->Read8Bit();
            result++;
            SetNFlag(result);
            SetZFlag(result);
            mode->Write8Bit(result);
            break;
        }

        case 0x3D: // INC X
        {
            LogInst("INC X");
            reg.x++;
            SetNFlag(reg.x);
            SetZFlag(reg.x);
            break;
        }

        case 0xFC: // INC Y
        {
            LogInst("INC Y");
            reg.y++;
            SetNFlag(reg.y);
            SetZFlag(reg.y);
            break;
        }

        case 0x3A: // INCW Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("INCW", &mode);
            uint16_t value = mode.Read16Bit() + 1;
            SetNFlag(value);
            SetZFlag(value);
            mode.Write16Bit(value);
            break;
        }

        case 0x9C: // DEC A
        case 0x8B: // DEC Direct
        case 0x9B: // DEC DirectIndexedX
        case 0x8C: // DEC Absolute
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("DEC", mode);
            uint8_t result = mode->Read8Bit();
            result--;
            SetNFlag(result);
            SetZFlag(result);
            mode->Write8Bit(result);
            break;
        }

        case 0x1D: // DEC X
        {
            LogInst("DEC X");
            reg.x--;
            SetNFlag(reg.x);
            SetZFlag(reg.x);
            break;
        }

        case 0xDC: // DEC Y
        {
            LogInst("DEC Y");
            reg.y--;
            SetNFlag(reg.y);
            SetZFlag(reg.y);
            break;
        }

        case 0x1A: // DECW Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            LogInstM("INCW", &mode);
            uint16_t value = mode.Read16Bit() - 1;
            SetNFlag(value);
            SetZFlag(value);
            mode.Write16Bit(value);
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Shift/Rotate opcodes                                                                                              //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ASL - Arithmetic Shift Left
        case 0x1C: // ASL A
        case 0x0B: // ASL Direct
        case 0x1B: // ASL DirectIndexedX
        case 0x0C: // ASL Absolute
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("ASL", mode);

            uint8_t value = mode->Read8Bit();
            uint8_t result = value << 1;

            reg.flags.c = value >> 7;
            SetNFlag(result);
            SetZFlag(result);
            mode->Write8Bit(result);
            break;
        }

        // LSR - Logical Shift Right
        case 0x5C: // LSR A
        case 0x4B: // LSR Direct
        case 0x5B: // LSR DirectIndexedX
        case 0x4C: // LSR Absolute
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("LSR", mode);

            uint8_t value = mode->Read8Bit();
            uint8_t result = value >> 1;

            reg.flags.c = value & 0x01;
            SetNFlag(result);
            SetZFlag(result);
            mode->Write8Bit(result);
            break;
        }

        // Rotate Left
        case 0x3C: // ROL A
        case 0x2B: // ROL Direct
        case 0x3B: // ROL DirectIndexedX
        case 0x2C: // ROL Absolute
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("ROL", mode);

            uint8_t value = mode->Read8Bit();
            uint8_t result = (value << 1) | reg.flags.c;

            reg.flags.c = value >> 7;
            SetNFlag(result);
            SetZFlag(result);
            mode->Write8Bit(result);
            break;
        }

        // Rotate Right
        case 0x7C: // ROR A
        case 0x6B: // ROR Direct
        case 0x7B: // ROR DirectIndexedX
        case 0x6C: // ROR Absolute
        {
            AddressModePtr &mode = addressModes[opcode & 0x1F];
            mode->LoadAddress();
            LogInstM("ROR", mode);

            uint8_t value = mode->Read8Bit();
            uint8_t result = (value >> 1) | (reg.flags.c << 7);

            reg.flags.c = value & 0x01;
            SetNFlag(result);
            SetZFlag(result);
            mode->Write8Bit(result);
            break;
        }

        case 0x9F: // XCN A
        {
            LogInst("XCN A");
            reg.a = (reg.a >> 4) | (reg.a << 4);
            SetNFlag(reg.a);
            SetZFlag(reg.a);
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Push/Pop opcodes                                                                                                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x2D: // PUSH A
        {
            LogInst("PUSH A");
            Push(reg.a);
            break;
        }

        case 0x4D: // PUSH X
        {
            LogInst("PUSH X");
            Push(reg.x);
            break;
        }

        case 0x6D: // PUSH Y
        {
            LogInst("PUSH Y");
            Push(reg.y);
            break;
        }

        case 0x0D: // PUSH PSW
        {
            LogInst("PUSH PSW");
            Push(reg.p);
            break;
        }

        case 0xAE: // POP A
        {
            LogInst("POP A");
            reg.a = Pop();
            break;
        }

        case 0xCE: // POP X
        {
            LogInst("POP X");
            reg.x = Pop();
            break;
        }

        case 0xEE: // POP Y
        {
            LogInst("POP Y");
            reg.y = Pop();
            break;
        }

        case 0x8E: // POP PSW
        {
            LogInst("POP PSW");
            reg.p = Pop();
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Branch opcodes                                                                                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x2F: // BRA
        {
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            LogSpc700("%02X %02X: BRA %d", opcode, (uint8_t)offset, offset);
            reg.pc += offset;
            break;
        }

        case 0x10: // BPL - Branch if Plus (n flag clear)
        case 0x30: // BMI - Branch if Minus (n flag set)
        case 0x50: // BVC - Branch if Overflow Clear
        case 0x70: // BVS - Branch if Overflow Set
        case 0x90: // BCC - Branch if Carry Clear
        case 0xB0: // BCS - Branch if Carry Set
        case 0xD0: // BNE - Branch if Not Equal (z flag clear)
        case 0xF0: // BEQ - Branch if Equal (z flag set)
        {
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            const char *names[] = {"BPL", "BMI", "BVC", "BVS", "BCC", "BCS", "BNE", "BEQ"};
            LogSpc700("%02X %02X: %s %d", opcode, (uint8_t)offset, names[opcode >> 5], offset);

            // Since you can't take the address of bitfield, a lookup table with pointers to the flags can't be used.
            // Instead, shift the p register until the desired bit is lsb. This is n, v, c, and z.
            uint8_t flagShift[] = {7, 6, 0, 1};

            // Bit 6 of the opcode says which flag to check, bit 5 is whether the flag should be set or cleared.
            if (((reg.p >> flagShift[opcode >> 6]) & 0x01) == ((opcode >> 5) & 0x01))
            {
                reg.pc += offset;
            }
            break;
        }

        case 0x03: // BBS 0
        case 0x23: // BBS 1
        case 0x43: // BBS 2
        case 0x63: // BBS 3
        case 0x83: // BBS 4
        case 0xA3: // BBS 5
        case 0xC3: // BBS 6
        case 0xE3: // BBS 7
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            uint8_t bit = opcode >> 5;
            LogSpc700("%02X %02X %02X: BBS %d", opcode, mode.GetAddress() & 0xFF, (uint8_t)offset, offset);

            uint8_t value = mode.Read8Bit();
            if (value & (1 << bit))
            {
                reg.pc += offset;
            }

            break;
        }

        case 0x13: // BBC 0
        case 0x33: // BBC 1
        case 0x53: // BBC 2
        case 0x73: // BBC 3
        case 0x93: // BBC 4
        case 0xB3: // BBC 5
        case 0xD3: // BBC 6
        case 0xF3: // BBC 7
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            uint8_t bit = opcode >> 5;
            LogSpc700("%02X %02X %02X: BBC %d", opcode, mode.GetAddress() & 0xFF, (uint8_t)offset, offset);

            uint8_t value = mode.Read8Bit();
            if (!(value & (1 << bit)))
            {
                reg.pc += offset;
            }

            break;
        }

        case 0x2E: // CBNE Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            LogSpc700("%02X %02X %02X: CBNE %d", opcode, mode.GetAddress() & 0xFF, (uint8_t)offset, offset);

            if (reg.a != mode.Read8Bit())
            {
                reg.pc += offset;
            }

            break;
        }

        case 0xDE: // CBNE DirectIndexedX
        {
            AddressModeDirectIndexedX mode(this, memory);
            mode.LoadAddress();
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            LogSpc700("%02X %02X %02X: CBNE %d", opcode, mode.GetAddress() & 0xFF, (uint8_t)offset, offset);

            if (reg.a != mode.Read8Bit())
            {
                reg.pc += offset;
            }

            break;
        }

        case 0x6E: // DBNZ Direct
        {
            AddressModeDirect mode(this, memory);
            mode.LoadAddress();
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            LogSpc700("%02X %02X %02X: DBNZ %d", opcode, mode.GetAddress() & 0xFF, (uint8_t)offset, offset);

            uint8_t value = mode.Read8Bit() - 1;
            mode.Write8Bit(value);
            if (value != 0)
            {
                reg.pc += offset;
            }

            break;
        }

        case 0xFE: // DBNZ Y
        {
            int8_t offset = static_cast<int8_t>(ReadPC8Bit());
            LogSpc700("%02X %02X: DBNZ Y %d", opcode, (uint8_t)offset, offset);

            reg.y--;
            if (reg.y != 0)
            {
                reg.pc += offset;
            }

            break;
        }

        case 0x5F: // JMP Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("JMP", &mode);

            reg.pc = mode.GetAddress();

            break;
        }

        case 0x1F: // JMP AbsoluteIndexedX
        {
            AddressModeAbsoluteIndexedX mode(this, memory);
            mode.LoadAddress();
            LogInstM("JMP", &mode);

            reg.pc = mode.Read16Bit();

            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Subroutine opcodes                                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x3F: // CALL Absolute
        {
            AddressModeAbsolute mode(this, memory);
            mode.LoadAddress();
            LogInstM("CALL", &mode);

            Push(reg.pc >> 8);
            Push(reg.pc);
            reg.pc = mode.GetAddress();

            break;
        }

        case 0x4F: // PCALL
        {
            AddressModeImmediate mode(this, memory);
            mode.LoadAddress();
            LogInstM("PCALL", &mode);

            Push(reg.pc >> 8);
            Push(reg.pc);
            reg.pc = 0xFF00 | mode.Read8Bit();

            break;
        }

        case 0x01: // TCALL 0
        case 0x11: // TCALL 1
        case 0x21: // TCALL 2
        case 0x31: // TCALL 3
        case 0x41: // TCALL 4
        case 0x51: // TCALL 5
        case 0x61: // TCALL 6
        case 0x71: // TCALL 7
        case 0x81: // TCALL 8
        case 0x91: // TCALL 9
        case 0xA1: // TCALL A
        case 0xB1: // TCALL B
        case 0xC1: // TCALL C
        case 0xD1: // TCALL D
        case 0xE1: // TCALL E
        case 0xF1: // TCALL F
        {
            uint8_t operand = opcode >> 4;
            LogInst("TCALL");

            Push(reg.pc >> 8);
            Push(reg.pc);
            reg.pc = memory->Read16Bit(0xFFDE - (2 * operand));

            break;
        }

        case 0x0F: // BRK
        {
            LogInst("BRK");

            Push(reg.pc >> 8);
            Push(reg.pc);
            Push(reg.p);
            reg.flags.b = 1;
            reg.flags.i = 0;
            reg.pc = memory->Read16Bit(0xFFDE);

            break;
        }

        case 0x6F: // RET
        {
            LogInst("RET");

            uint8_t pcl = Pop();
            uint8_t pch = Pop();
            reg.pc = (pch << 8) | pcl;

            break;
        }

        case 0x7F: // RETI
        {
            LogInst("RETI");

            reg.p = Pop();
            uint8_t pcl = Pop();
            uint8_t pch = Pop();
            reg.pc = (pch << 8) | pcl;

            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Flag opcodes                                                                                                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x60: // CLRC
        {
            LogInst("CLRC");
            reg.flags.c = 0;
            break;
        }

        case 0x80: // SETC
        {
            LogInst("SETC");
            reg.flags.c = 1;
            break;
        }

        case 0xED: // NOTC
        {
            LogInst("NOTC");
            reg.flags.c ^= 1;
            break;
        }

        case 0xE0: // CLRV
        {
            LogInst("CLRV");
            reg.flags.v = 0;
            reg.flags.h = 0;
            break;
        }

        case 0x20: // CLRP
        {
            LogInst("CLRP");
            reg.flags.p = 0;
            break;
        }

        case 0x40: // SETP
        {
            LogInst("SETP");
            reg.flags.p = 1;
            break;
        }

        case 0xA0: // EI
        {
            LogInst("EI");
            reg.flags.i = 1;
            break;
        }

        case 0xC0: // DI
        {
            LogInst("DI");
            reg.flags.i = 0;
            break;
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Misc opcodes                                                                                                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0xDF: // DAA
        {
            LogInst("DAA");
            if (reg.a > 0x99 || reg.flags.c)
            {
                reg.a += 0x60;
                reg.flags.c = 1;
            }
            if ((reg.a & 0x0F) > 0x09 || reg.flags.h)
            {
                reg.a += 0x06;
            }
            SetNFlag(reg.a);
            SetZFlag(reg.a);
            break;
        }

        case 0xBE: // DAS
        {
            LogInst("DAS");
            if (reg.a > 0x99 || !reg.flags.c)
            {
                reg.a -= 0x60;
                reg.flags.c = 0;
            }
            if ((reg.a & 0x0F) > 0x09 || !reg.flags.h)
            {
                reg.a -= 0x06;
            }
            SetNFlag(reg.a);
            SetZFlag(reg.a);
            break;
        }

        case 0x00: // NOP
        {
            LogInst("NOP");
            break;
        }

        case 0xEF: // SLEEP
        {
            LogInst("SLEEP");
            waiting = true;
        }
        break;

        case 0xFF: // STOP
        {
            LogInst("STOP");
            waiting = true;
        }
        break;
    }

    PrintState();
}


} // end namespace