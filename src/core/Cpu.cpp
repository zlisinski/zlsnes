#include <sstream>

#include "AddressMode.h"
#include "Bcd.h"
#include "Cpu.h"
#include "Memory.h"


Cpu::Cpu(Memory *memory) :
    reg(),
    memory(memory)
{
    addressModes[0x00] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x01] = std::make_unique<AddressModeDirectIndexedIndirect>(this, memory);
    addressModes[0x02] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x03] = std::make_unique<AddressModeStackRelative>(this, memory);
    addressModes[0x04] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x05] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x06] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x07] = std::make_unique<AddressModeDirectIndirectLong>(this, memory);
    // No 0x08
    addressModes[0x09] = std::make_unique<AddressModeImmediate>(this, memory);
    addressModes[0x0A] = std::make_unique<AddressModeAccumulator>(this, memory);
    // No 0x0B
    addressModes[0x0C] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0D] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0E] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0F] = std::make_unique<AddressModeAbsoluteLong>(this, memory);
    // No 0x10
    addressModes[0x11] = std::make_unique<AddressModeDirectIndirectIndexed>(this, memory);
    addressModes[0x12] = std::make_unique<AddressModeDirectIndirect>(this, memory);
    addressModes[0x13] = std::make_unique<AddressModeStackRelativeIndirectIndexed>(this, memory);
    addressModes[0x14] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x15] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x16] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x17] = std::make_unique<AddressModeDirectIndirectLongIndexed>(this, memory);
    // No 0x18
    addressModes[0x19] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory);
    addressModes[0x1A] = std::make_unique<AddressModeAccumulator>(this, memory);
    // No 0x1B
    addressModes[0x1C] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1D] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1E] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1F] = std::make_unique<AddressModeAbsoluteLongIndexedX>(this, memory);

    // Special cases for certain opcodes of LDX, STX, STZ
    addressModeAlternate[0x16] = std::make_unique<AddressModeDirectIndexedY>(this, memory); // LDX, STX
    addressModeAlternate[0x1C] = std::make_unique<AddressModeAbsolute>(this, memory); // STZ
    addressModeAlternate[0x1E] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory); // LDX
}

Cpu::~Cpu()
{

}

uint8_t Cpu::ReadPC8Bit()
{
    uint8_t byte = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    //timer->AddCycle();

    reg.pc++;

    return byte;
}


uint16_t Cpu::ReadPC16Bit()
{
    uint8_t low = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;
    //timer->AddCycle();

    uint8_t high = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;
    //timer->AddCycle();

    uint16_t word = (high << 8) | low;

    return word;
}


uint32_t Cpu::ReadPC24Bit()
{
    uint8_t low = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;
    //timer->AddCycle();

    uint8_t mid = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;
    //timer->AddCycle();

    uint8_t high = memory->Read8Bit(Bytes::Make24Bit(reg.pb, reg.pc));
    reg.pc++;
    //timer->AddCycle();

    uint32_t word = (high << 16) | (mid << 8) | low;

    return word;
}


void Cpu::ProcessOpCode()
{
    uint8_t opcode = ReadPC8Bit();

    switch (opcode)
    {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Register to register transfer opcodes                                                                             //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0xAA: // TAX - Transfer A to X.
            {
                LogInstruction("%02X: TAX", opcode);
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, reg.a);
                else
                    LoadRegister(&reg.xl, reg.al);
            }
            break;

        case 0xA8: // TAY - Transfer A to Y.
            {
                LogInstruction("%02X: TAY", opcode);
                if (IsIndex16Bit())
                    LoadRegister(&reg.y, reg.a);
                else
                    LoadRegister(&reg.yl, reg.al);
            }
            break;

        case 0xBA: // TSX - Transfer SP to X.
            {
                LogInstruction("%02X: TSX", opcode);
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, reg.sp);
                else
                    LoadRegister(&reg.xl, reg.sl);
            }
            break;

        case 0x8A: // TXA - Transfer X to A.
            {
                LogInstruction("%02X: TXA", opcode);
                if (IsAccumulator16Bit())
                    LoadRegister(&reg.a, reg.x);
                else
                    LoadRegister(&reg.al, reg.xl);
            }
            break;

        case 0x9A: // TXS - Transfer X to SP.
            {
                LogInstruction("%02X: TXS", opcode);

                // No flags are set. High byte of sp is always 0x01 in emulation mode.
                if (reg.emulationMode)
                    reg.sp = 0x0100 | reg.xl;
                else
                    reg.sp = reg.x;
            }
            break;

        case 0x9B: // TXY - Transfer X to Y.
            {
                LogInstruction("%02X: TXY", opcode);
                if (IsIndex16Bit())
                    LoadRegister(&reg.y, reg.x);
                else
                    LoadRegister(&reg.yl, reg.xl);
            }
            break;

        case 0x98: // TYA - Transfer Y to A.
            {
                LogInstruction("%02X: TYA", opcode);
                if (IsAccumulator16Bit())
                    LoadRegister(&reg.a, reg.y);
                else
                    LoadRegister(&reg.al, reg.yl);
            }
            break;

        case 0xBB: // TYX - Transfer Y to X.
            {
                LogInstruction("%02X: TYX", opcode);
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, reg.y);
                else
                    LoadRegister(&reg.xl, reg.yl);
            }
            break;

        case 0x5B: // TCD/TAD - Transfer A to D.
            {
                LogInstruction("%02X: TCD", opcode);
                LoadRegister(&reg.d, reg.a);
            }
            break;

        case 0x1B: // TCS/TAS - Transfer A to SP.
            {
                LogInstruction("%02X: TCS", opcode);

                // No flags are set. High byte of sp is always 0x01 in emulation mode.
                if (reg.emulationMode)
                    reg.sp = 0x0100 | reg.al;
                else
                    reg.sp = reg.a;
            }
            break;

        case 0x7B: // TDC/TDA - Transfer D to A.
            {
                LogInstruction("%02X: TDC", opcode);
                LoadRegister(&reg.a, reg.d);
            }
            break;

        case 0x3B: // TSC/TSA - Transfer SP to A.
            {
                LogInstruction("%02X: TSC", opcode);
                LoadRegister(&reg.a, reg.sp);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Load opcodes                                                                                                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // LDA - Load A
        case 0xA1: // LDA (Direct,X)
        case 0xA3: // LDA Stack,S
        case 0xA5: // LDA Direct
        case 0xA7: // LDA [Direct]
        case 0xA9: // LDA Immediate
        case 0xAD: // LDA Absolute
        case 0xAF: // LDA Long
        case 0xB1: // LDA (Direct),Y
        case 0xB2: // LDA (Direct)
        case 0xB3: // LDA (Stack,S),Y
        case 0xB5: // LDA Direct,X
        case 0xB7: // LDA [Direct],Y
        case 0xB9: // LDA Absolute,Y
        case 0xBD: // LDA Absolute,X
        case 0xBF: // LDA Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                    LoadRegister(&reg.a, mode->Read16Bit());
                else
                    LoadRegister(&reg.al, mode->Read8Bit());
            }
            break;

        // LDX - Load X
        case 0xA2: // LDX Immediate
        case 0xA6: // LDX Direct
        case 0xAE: // LDX Absolute
        case 0xB6: // LDX Direct,Y
        case 0xBE: // LDX Absolute,Y
            {
                // Opcodes 0xB6 and 0xBE are special cases.
                AddressModePtr &mode = (opcode & 0x10 ? addressModeAlternate[opcode & 0x1F] : addressModes[opcode & 0x1F]);
                mode->LoadAddress();
                if (IsIndex16Bit())
                    LoadRegister(&reg.x, mode->Read16Bit());
                else
                    LoadRegister(&reg.xl, mode->Read8Bit());
            }
            break;

        // LDY - Load Y
        case 0xA0: // LDY Immediate
        case 0xA4: // LDY Direct
        case 0xAC: // LDY Absolute
        case 0xB4: // LDY Direct,X
        case 0xBC: // LDY Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsIndex16Bit())
                    LoadRegister(&reg.y, mode->Read16Bit());
                else
                    LoadRegister(&reg.yl, mode->Read8Bit());
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Store opcodes                                                                                                     //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // STA - Store A
        case 0x81: // STA (Direct,X)
        case 0x83: // STA Stack,S
        case 0x85: // STA Direct
        case 0x87: // STA [Direct]
        case 0x8D: // STA Absolute
        case 0x8F: // STA Long
        case 0x91: // STA (Direct),Y
        case 0x92: // STA (Direct)
        case 0x95: // STA Direct,X
        case 0x97: // STA [Direct],Y
        case 0x99: // STA Absolute,Y
        case 0x9D: // STA Absolute,X
        case 0x9F: // STA Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                    mode->Write16Bit(reg.a);
                else
                    mode->Write8Bit(reg.al);
            }
            break;

        // STX - Store X
        case 0x86: // STX Direct
        case 0x8E: // STX Absolute
        case 0x96: // STX Direct,Y
            {
                // Opcodes 0x96 is a special case.
                AddressModePtr &mode = (opcode == 0x96 ? addressModeAlternate[opcode & 0x1F] : addressModes[opcode & 0x1F]);
                mode->LoadAddress();
                if (IsIndex16Bit())
                    mode->Write16Bit(reg.x);
                else
                    mode->Write8Bit(reg.xl);
            }
            break;

        // STY - Store Y
        case 0x84: // STY Direct
        case 0x8C: // STY Absolute
        case 0x94: // STY Direct,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsIndex16Bit())
                    mode->Write16Bit(reg.y);
                else
                    mode->Write8Bit(reg.yl);
            }
            break;

        // STZ - Store Zero
        case 0x64: // STZ Direct
        case 0x74: // STZ Direct,X
        case 0x9C: // STZ Absolute
        case 0x9E: // STZ Absolute,X
            {
                // Opcodes 0x9C is a special case.
                AddressModePtr &mode = (opcode == 0x9C ? addressModeAlternate[opcode & 0x1F] : addressModes[opcode & 0x1F]);
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                    mode->Write16Bit(0);
                else
                    mode->Write8Bit(0);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Stack opcodes                                                                                                     //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x48: // PHA - Push A
            {
                LogInstruction("%02X: PHA", opcode);
                if (IsAccumulator16Bit())
                    Push16Bit(reg.a);
                else
                    Push8Bit(reg.al);
            }
            break;

        case 0xDA: // PHX - Push X
            {
                LogInstruction("%02X: PHX", opcode);
                if (IsIndex16Bit())
                    Push16Bit(reg.x);
                else
                    Push8Bit(reg.xl);
            }
            break;

        case 0x5A: // PHY - Push Y
            {
                LogInstruction("%02X: PHY", opcode);
                if (IsIndex16Bit())
                    Push16Bit(reg.y);
                else
                    Push8Bit(reg.yl);
            }
            break;

        case 0x8B: // PHB - Push DB
            {
                LogInstruction("%02X: PHB", opcode);
                Push8Bit(reg.db);
            }
            break;

        case 0x0B: // PHD - Push D
            {
                LogInstruction("%02X: PHD", opcode);
                Push16Bit(reg.d);
            }
            break;

        case 0x4B: // PHK - Push PB
            {
                LogInstruction("%02X: PHK", opcode);
                Push8Bit(reg.pb);
            }
            break;

        case 0x08: // PHP - Push P
            {
                LogInstruction("%02X: PHP", opcode);
                Push8Bit(reg.p);
            }
            break;

        case 0xF4: // PEA - Push Effective Address
            {
                uint16_t value = ReadPC16Bit();
                LogInstruction("%02X %02X %02X: PEA", opcode, Bytes::GetByte<1>(value), Bytes::GetByte<0>(value));
                Push16Bit(value);
            }
            break;

        case 0xD4: // PEI - Push Effective Indirect Address
            {
                AddressModeDirect mode(this, memory);
                mode.LoadAddress();
                LogInstruction("%02X: PEI", opcode);
                Push16Bit(mode.Read16Bit());
            }
            break;

        case 0x62: // PER - Push Effective Relative Address
            {
                int16_t value = static_cast<int16_t>(ReadPC16Bit());
                LogInstruction("%02X %02X %02X: PER", opcode, Bytes::GetByte<1>(value), Bytes::GetByte<0>(value));
                Push16Bit(reg.pc + value);
            }
            break;

        case 0x68: // PLA - Pull/Pop A
            {
                LogInstruction("%02X: PLA", opcode);
                if (IsAccumulator16Bit())
                {
                    reg.a = Pop16Bit();
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    reg.al = Pop8Bit();
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        case 0xFA: // PLX - Pull/Pop X
            {
                LogInstruction("%02X: PLX", opcode);
                if (IsIndex16Bit())
                {
                    reg.x = Pop16Bit();
                    SetNFlag(reg.x);
                    SetZFlag(reg.x);
                }
                else
                {
                    reg.xl = Pop8Bit();
                    SetNFlag(reg.xl);
                    SetZFlag(reg.xl);
                }
            }
            break;

        case 0x7A: // PLY - Pull/Pop Y
            {
                LogInstruction("%02X: PLY", opcode);
                if (IsIndex16Bit())
                {
                    reg.y = Pop16Bit();
                    SetNFlag(reg.y);
                    SetZFlag(reg.y);
                }
                else
                {
                    reg.yl = Pop8Bit();
                    SetNFlag(reg.yl);
                    SetZFlag(reg.yl);
                }
            }
            break;

        case 0xAB: // PLB - Pull/Pop DB
            {
                LogInstruction("%02X: PLB", opcode);
                reg.db = Pop8Bit();
                SetNFlag(reg.db);
                SetZFlag(reg.db);
            }
            break;

        case 0x2B: // PLD - Pull/Pop D
            {
                LogInstruction("%02X: PLD", opcode);
                reg.d = Pop16Bit();
                SetNFlag(reg.d);
                SetZFlag(reg.d);
            }
            break;

        case 0x28: // PLP - Pull/Pop P
            {
                LogInstruction("%02X: PLP", opcode);
                reg.p = Pop8Bit();
                if (IsIndex8Bit())
                {
                    reg.x &= 0xFF;
                    reg.y &= 0xFF;
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Logical opcodes                                                                                                   //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // AND - Bitwise AND
        case 0x21: // AND (Direct,X)
        case 0x23: // AND Stack,S
        case 0x25: // AND Direct
        case 0x27: // AND [Direct]
        case 0x29: // AND Immediate
        case 0x2D: // AND Absolute
        case 0x2F: // AND Long
        case 0x31: // AND (Direct),Y
        case 0x32: // AND (Direct)
        case 0x33: // AND (Stack,S),Y
        case 0x35: // AND Direct,X
        case 0x37: // AND [Direct],Y
        case 0x39: // AND Absolute,Y
        case 0x3D: // AND Absolute,X
        case 0x3F: // AND Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    reg.a &= value;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.al &= value;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        // EOR - Bitwise Exclusive OR
        case 0x41: // EOR (Direct,X)
        case 0x43: // EOR Stack,S
        case 0x45: // EOR Direct
        case 0x47: // EOR [Direct]
        case 0x49: // EOR Immediate
        case 0x4D: // EOR Absolute
        case 0x4F: // EOR Long
        case 0x51: // EOR (Direct),Y
        case 0x52: // EOR (Direct)
        case 0x53: // EOR (Stack,S),Y
        case 0x55: // EOR Direct,X
        case 0x57: // EOR [Direct],Y
        case 0x59: // EOR Absolute,Y
        case 0x5D: // EOR Absolute,X
        case 0x5F: // EOR Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    reg.a ^= value;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.al ^= value;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        // ORA - Bitwise OR Accumulator
        case 0x01: // ORA (Direct,X)
        case 0x03: // ORA Stack,S
        case 0x05: // ORA Direct
        case 0x07: // ORA [Direct]
        case 0x09: // ORA Immediate
        case 0x0D: // ORA Absolute
        case 0x0F: // ORA Long
        case 0x11: // ORA (Direct),Y
        case 0x12: // ORA (Direct)
        case 0x13: // ORA (Stack,S),Y
        case 0x15: // ORA Direct,X
        case 0x17: // ORA [Direct],Y
        case 0x19: // ORA Absolute,Y
        case 0x1D: // ORA Absolute,X
        case 0x1F: // ORA Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    reg.a |= value;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.al |= value;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Arithmetic opcodes                                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ADC - Add with Carry
        case 0x61: // ADC (Direct,X)
        case 0x63: // ADC Stack,S
        case 0x65: // ADC Direct
        case 0x67: // ADC [Direct]
        case 0x69: // ADC Immediate
        case 0x6D: // ADC Absolute
        case 0x6F: // ADC Long
        case 0x71: // ADC (Direct),Y
        case 0x72: // ADC (Direct)
        case 0x73: // ADC (Stack,S),Y
        case 0x75: // ADC Direct,X
        case 0x77: // ADC [Direct],Y
        case 0x79: // ADC Absolute,Y
        case 0x7D: // ADC Absolute,X
        case 0x7F: // ADC Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t operand = mode->Read16Bit();
                    uint32_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.a + reg.flags.c + operand;
                    }
                    else
                    {
                        result = Bcd::Add(reg.a, operand);
                        result = Bcd::Add(result, reg.flags.c);
                    }

                    reg.flags.c = result > 0xFFFF;
                    reg.flags.v = (((reg.a ^ result) & ~(reg.a ^ operand)) & 0x8000) != 0;
                    reg.a = result;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t operand = mode->Read8Bit();
                    uint16_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.al + reg.flags.c + operand;
                    }
                    else
                    {
                        result = Bcd::Add(reg.al, operand);
                        result = Bcd::Add(result, reg.flags.c);
                    }

                    reg.flags.c = result > 0xFF;
                    reg.flags.v = (((reg.al ^ result) & ~(reg.al ^ operand)) & 0x80) != 0;
                    reg.al = result;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        // SBC - Subtract with Carry
        case 0xE1: // SBC (Direct,X)
        case 0xE3: // SBC Stack,S
        case 0xE5: // SBC Direct
        case 0xE7: // SBC [Direct]
        case 0xE9: // SBC Immediate
        case 0xED: // SBC Absolute
        case 0xEF: // SBC Long
        case 0xF1: // SBC (Direct),Y
        case 0xF2: // SBC (Direct)
        case 0xF3: // SBC (Stack,S),Y
        case 0xF5: // SBC Direct,X
        case 0xF7: // SBC [Direct],Y
        case 0xF9: // SBC Absolute,Y
        case 0xFD: // SBC Absolute,X
        case 0xFF: // SBC Long,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t operand = ~mode->Read16Bit();
                    uint32_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.a + reg.flags.c + operand;
                    }
                    else
                    {
                        result = Bcd::Subtract(reg.a, operand);
                        result = Bcd::Subtract(result, !reg.flags.c);
                    }

                    reg.flags.c = result > 0xFFFF;
                    reg.flags.v = (((reg.a ^ result) & ~(reg.a ^ operand)) & 0x8000) != 0;
                    reg.a = result;
                    SetNFlag(reg.a);
                    SetZFlag(reg.a);
                }
                else
                {
                    uint8_t operand = ~mode->Read8Bit();
                    uint16_t result;

                    if (reg.flags.d == 0)
                    {
                        result = reg.al + reg.flags.c + operand;
                    }
                    else
                    {
                        result = Bcd::Subtract(reg.al, operand);
                        result = Bcd::Subtract(result, !reg.flags.c);
                    }

                    reg.flags.c = result > 0xFF;
                    reg.flags.v = (((reg.al ^ result) & ~(reg.al ^ operand)) & 0x80) != 0;
                    reg.al = result;
                    SetNFlag(reg.al);
                    SetZFlag(reg.al);
                }
            }
            break;

        case 0x3A: // DEC Accumulator
        case 0xC6: // DEC Direct
        case 0xCE: // DEC Absolute
        case 0xD6: // DEC Direct,X
        case 0xDE: // DEC Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstruction("%02X: DEC", opcode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    value--;
                    mode->Write16Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    value--;
                    mode->Write8Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
            }
            break;

        case 0xCA: // DEX
            {
                LogInstruction("%02X: DEX", opcode);
                if (IsIndex16Bit())
                {
                    reg.x--;
                    SetNFlag(reg.x);
                    SetZFlag(reg.x);
                }
                else
                {
                    reg.xl--;
                    SetNFlag(reg.xl);
                    SetZFlag(reg.xl);
                }
            }
            break;

        case 0x88: // DEY
            {
                LogInstruction("%02X: DEY", opcode);
                if (IsIndex16Bit())
                {
                    reg.y--;
                    SetNFlag(reg.y);
                    SetZFlag(reg.y);
                }
                else
                {
                    reg.yl--;
                    SetNFlag(reg.yl);
                    SetZFlag(reg.yl);
                }
            }
            break;

        case 0x1A: // INC Accumulator
        case 0xE6: // INC Direct
        case 0xEE: // INC Absolute
        case 0xF6: // INC Direct,X
        case 0xFE: // INC Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                LogInstruction("%02X: INC", opcode);

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    value++;
                    mode->Write16Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    value++;
                    mode->Write8Bit(value);
                    SetNFlag(value);
                    SetZFlag(value);
                }
            }
            break;

        case 0xE8: // INX
            {
                LogInstruction("%02X: INX", opcode);
                if (IsIndex16Bit())
                {
                    reg.x++;
                    SetNFlag(reg.x);
                    SetZFlag(reg.x);
                }
                else
                {
                    reg.xl++;
                    SetNFlag(reg.xl);
                    SetZFlag(reg.xl);
                }
            }
            break;

        case 0xC8: // INY
            {
                LogInstruction("%02X: INY", opcode);
                if (IsIndex16Bit())
                {
                    reg.y++;
                    SetNFlag(reg.y);
                    SetZFlag(reg.y);
                }
                else
                {
                    reg.yl++;
                    SetNFlag(reg.yl);
                    SetZFlag(reg.yl);
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Comparison opcodes                                                                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // CMP - Compare to Accumulator
        case 0xC1: // CMP (Direct,X)
        case 0xC3: // CMP Stack,S
        case 0xC5: // CMP Direct
        case 0xC7: // CMP [Direct]
        case 0xC9: // CMP Immediate
        case 0xCD: // CMP Absolute
        case 0xCF: // CMP Long
        case 0xD1: // CMP (Direct),Y
        case 0xD2: // CMP (Direct)
        case 0xD3: // CMP (Stack,S),Y
        case 0xD5: // CMP Direct,X
        case 0xD7: // CMP [Direct],Y
        case 0xD9: // CMP Absolute,Y
        case 0xDD: // CMP Absolute,X
        case 0xDF: // CMP Long,X
            {
                LogInstruction("%02X: CMP", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsAccumulator16Bit())
                    Compare(reg.a, mode->Read16Bit());
                else
                    Compare(reg.al, mode->Read8Bit());
            }
            break;

        // CPX - Compare to X
        case 0xE0: // CPX Immediate
        case 0xE4: // CPX Direct
        case 0xEC: // CPX Absolute
            {
                LogInstruction("%02X: CPX", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsIndex16Bit())
                    Compare(reg.x, mode->Read16Bit());
                else
                    Compare(reg.xl, mode->Read8Bit());
            }
            break;

        // CPY - Compare to Y
        case 0xC0: // CPY Immediate
        case 0xC4: // CPY Direct
        case 0xCC: // CPY Absolute
            {
                LogInstruction("%02X: CPY", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsIndex16Bit())
                    Compare(reg.y, mode->Read16Bit());
                else
                    Compare(reg.yl, mode->Read8Bit());
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Bit test/set/reset opcodes                                                                                        //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // BIT - Test Bit
        case 0x24: // BIT Direct
        case 0x2C: // BIT Absolute
        case 0x34: // BIT Direct,X
        case 0x3C: // BIT Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = reg.a & value;

                    // N and V flag look at data, not result.
                    SetNFlag(value);
                    // V looks at the second highest bit.
                    reg.flags.v = (value & 0x4000) != 0;
                    SetZFlag(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = reg.al & value;

                    // N and V flag look at data, not result.
                    SetNFlag(value);
                    // V looks at the second highest bit.
                    reg.flags.v = (value & 0x40) != 0;
                    SetZFlag(result);
                }
            }
            break;

        case 0x89: // BIT Immediate
            {
                AddressModeImmediate mode(this, memory);
                mode.LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode.Read16Bit();
                    uint16_t result = reg.a & value;

                    // N and V flags are not changed.
                    SetZFlag(result);
                }
                else
                {
                    uint8_t value = mode.Read8Bit();
                    uint8_t result = reg.al & value;

                    // N and V flags are not changed.
                    SetZFlag(result);
                }
            }
            break;

        // TRB - Test and Reset Bit
        case 0x14: // TRB Direct
        case 0x1C: // TRB Absolute
            {
                // TRB is a special case, use 0x0F for masking.
                AddressModePtr &mode = addressModes[opcode & 0x0F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = ~reg.a & value;

                    // Z flag is based on reg.a AND value.
                    SetZFlag(static_cast<uint16_t>(reg.a & value));
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = ~reg.al & value;

                    // Z flag is based on reg.al AND value.
                    SetZFlag(static_cast<uint8_t>(reg.al & value));
                    mode->Write8Bit(result);
                }
            }
            break;

        // TSB - Test and Set Bit
        case 0x04: // TSB Direct
        case 0x0C: // TSB Absolute
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = reg.a | value;

                    // Z flag is based on reg.a AND value.
                    SetZFlag(static_cast<uint16_t>(reg.a & value));
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = reg.al | value;

                    // Z flag is based on reg.al AND value.
                    SetZFlag(static_cast<uint8_t>(reg.al & value));
                    mode->Write8Bit(result);
                }
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Shift and Rotate opcodes                                                                                          //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ASL - Arithmetic Shift Left
        case 0x06: // ASL Direct
        case 0x0A: // ASL Acc
        case 0x0E: // ASL Absolute
        case 0x16: // ASL Direct,X
        case 0x1E: // ASL Absolute,X
            {
                LogInstruction("%02X: ASL", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = value << 1;

                    reg.flags.c = value >> 15;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = value << 1;

                    reg.flags.c = value >> 7;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        // LSR - Logical Shift Right
        case 0x46: // LSR Direct
        case 0x4A: // LSR Acc
        case 0x4E: // LSR Absolute
        case 0x56: // LSR Direct,X
        case 0x5E: // LSR Absolute,X
            {
                LogInstruction("%02X: LSR", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = value >> 1;

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = value >> 1;

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        // ROL - Rotate Left
        case 0x26: // ROL Direct
        case 0x2A: // ROL Acc
        case 0x2E: // ROL Absolute
        case 0x36: // ROL Direct,X
        case 0x3E: // ROL Absolute,X
            {
                LogInstruction("%02X: ROL", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = (value << 1) | reg.flags.c;

                    reg.flags.c = value >> 15;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = (value << 1) | reg.flags.c;

                    reg.flags.c = value >> 7;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        // ROR - Rotate Right
        case 0x66: // ROR Direct
        case 0x6A: // ROR Acc
        case 0x6E: // ROR Absolute
        case 0x76: // ROR Direct,X
        case 0x7E: // ROR Absolute,X
            {
                LogInstruction("%02X: ROR", opcode);
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();

                if (IsAccumulator16Bit())
                {
                    uint16_t value = mode->Read16Bit();
                    uint16_t result = (value >> 1) | (reg.flags.c << 15);

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write16Bit(result);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    uint8_t result = (value >> 1) | (reg.flags.c << 7);

                    reg.flags.c = value & 0x01;
                    SetNFlag(result);
                    SetZFlag(result);
                    mode->Write8Bit(result);
                }
            }
            break;

        case 0x00: NotYetImplemented(0x00); break;
        case 0x02: NotYetImplemented(0x02); break;

        case 0x10: NotYetImplemented(0x10); break;
        case 0x18: NotYetImplemented(0x18); break;

        case 0x20: NotYetImplemented(0x20); break;
        case 0x22: NotYetImplemented(0x22); break;

        case 0x30: NotYetImplemented(0x30); break;
        case 0x38: NotYetImplemented(0x38); break;

        case 0x40: NotYetImplemented(0x40); break;
        case 0x42: NotYetImplemented(0x42); break;
        case 0x44: NotYetImplemented(0x44); break;
        case 0x4C: NotYetImplemented(0x4C); break;

        case 0x50: NotYetImplemented(0x50); break;
        case 0x54: NotYetImplemented(0x54); break;
        case 0x58: NotYetImplemented(0x58); break;
        case 0x5C: NotYetImplemented(0x5C); break;

        case 0x60: NotYetImplemented(0x60); break;
        case 0x6B: NotYetImplemented(0x6B); break;
        case 0x6C: NotYetImplemented(0x6C); break;

        case 0x70: NotYetImplemented(0x70); break;
        case 0x78: NotYetImplemented(0x78); break;
        case 0x7C: NotYetImplemented(0x7C); break;

        case 0x80: NotYetImplemented(0x80); break;
        case 0x82: NotYetImplemented(0x82); break;

        case 0x90: NotYetImplemented(0x90); break;
        case 0x93: NotYetImplemented(0x93); break;

        case 0xB0: NotYetImplemented(0xB0); break;
        case 0xB8: NotYetImplemented(0xB8); break;

        case 0xC2: NotYetImplemented(0xC2); break;
        case 0xCB: NotYetImplemented(0xCB); break;

        case 0xD0: NotYetImplemented(0xD0); break;
        case 0xD8: NotYetImplemented(0xD8); break;
        case 0xDB: NotYetImplemented(0xDB); break;
        case 0xDC: NotYetImplemented(0xDC); break;

        case 0xE2: NotYetImplemented(0xE2); break;
        case 0xEA: NotYetImplemented(0xEA); break;
        case 0xEB: NotYetImplemented(0xEB); break;

        case 0xF0: NotYetImplemented(0xF0); break;
        case 0xF8: NotYetImplemented(0xF8); break;
        case 0xFB: NotYetImplemented(0xFB); break;
        case 0xFC: NotYetImplemented(0xFC); break;
    }
}


void Cpu::NotYetImplemented(uint8_t opcode)
{
    // reg.pc is advanced in ReadPC8Bit, so subtract 1 to get the real address of the error.
    uint32_t addr = GetFullPC(reg.pc - 1);

    std::stringstream ss;
    ss << "NYI opcode 0x" << std::hex << std::uppercase << (int)opcode << " at 0x" << (int)addr;
    throw NotYetImplementedException(ss.str());
}