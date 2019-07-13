#include <sstream>

#include "AddressMode.h"
#include "Cpu.h"
#include "Memory.h"


Cpu::Cpu(Memory *memory) :
    reg(),
    memory(memory)
{
    // Used by LDA, STA, ORA, AND, EOR, ADC, SBC, CMP, CPX, CPY
    addressModes[0x01] = std::make_unique<AddressModeDirectIndexedIndirect>(this, memory);
    addressModes[0x03] = std::make_unique<AddressModeStackRelative>(this, memory);
    addressModes[0x05] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x07] = std::make_unique<AddressModeDirectIndirectLong>(this, memory);
    addressModes[0x0D] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0F] = std::make_unique<AddressModeAbsoluteLong>(this, memory);
    addressModes[0x11] = std::make_unique<AddressModeDirectIndirectIndexed>(this, memory);
    addressModes[0x12] = std::make_unique<AddressModeDirectIndirect>(this, memory);
    addressModes[0x13] = std::make_unique<AddressModeStackRelativeIndirectIndexed>(this, memory);
    addressModes[0x15] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x17] = std::make_unique<AddressModeDirectIndirectLongIndexed>(this, memory);
    addressModes[0x19] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory);
    addressModes[0x1D] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1F] = std::make_unique<AddressModeAbsoluteLongIndexedX>(this, memory);

    // Used by LDX, LDY, STX, STY, STZ
    addressModes[0x04] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x06] = std::make_unique<AddressModeDirect>(this, memory);
    addressModes[0x0C] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x0E] = std::make_unique<AddressModeAbsolute>(this, memory);
    addressModes[0x14] = std::make_unique<AddressModeDirectIndexedX>(this, memory);
    addressModes[0x16] = std::make_unique<AddressModeDirectIndexedY>(this, memory);
    addressModes[0x1C] = std::make_unique<AddressModeAbsoluteIndexedX>(this, memory);
    addressModes[0x1E] = std::make_unique<AddressModeAbsoluteIndexedY>(this, memory);
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
                    LoadRegister16Bit(&reg.x, reg.a);
                else
                    LoadRegister8Bit(&reg.x, reg.a);
            }
            break;

        case 0xA8: // TAY - Transfer A to Y.
            {
                LogInstruction("%02X: TAY", opcode);
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.y, reg.a);
                else
                    LoadRegister8Bit(&reg.y, reg.a);
            }
            break;

        case 0xBA: // TSX - Transfer SP to X.
            {
                LogInstruction("%02X: TSX", opcode);
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.x, reg.sp);
                else
                    LoadRegister8Bit(&reg.x, reg.sp);
            }
            break;

        case 0x8A: // TXA - Transfer X to A.
            {
                LogInstruction("%02X: TXA", opcode);
                if (IsAccumulator16Bit())
                    LoadRegister16Bit(&reg.a, reg.x);
                else
                    LoadRegister8Bit(&reg.a, reg.x);
            }
            break;

        case 0x9A: // TXS - Transfer X to SP.
            {
                LogInstruction("%02X: TXS", opcode);

                // No flags are set. High byte of sp is always 0x01 in emulation mode.
                if (reg.emulationMode)
                    reg.sp = 0x0100 | Bytes::MaskByte<0>(reg.x);
                else
                    reg.sp = reg.x;
            }
            break;

        case 0x9B: // TXY - Transfer X to Y.
            {
                LogInstruction("%02X: TXY", opcode);
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.y, reg.x);
                else
                    LoadRegister8Bit(&reg.y, reg.x);
            }
            break;

        case 0x98: // TYA - Transfer Y to A.
            {
                LogInstruction("%02X: TYA", opcode);
                if (IsAccumulator16Bit())
                    LoadRegister16Bit(&reg.a, reg.y);
                else
                    LoadRegister8Bit(&reg.a, reg.y);
            }
            break;

        case 0xBB: // TYX - Transfer Y to X.
            {
                LogInstruction("%02X: TYX", opcode);
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.x, reg.y);
                else
                    LoadRegister8Bit(&reg.x, reg.y);
            }
            break;

        case 0x5B: // TCD/TAD - Transfer A to D.
            {
                LogInstruction("%02X: TCD", opcode);
                LoadRegister16Bit(&reg.d, reg.a);
            }
            break;

        case 0x1B: // TCS/TAS - Transfer A to SP.
            {
                LogInstruction("%02X: TCS", opcode);

                // No flags are set. High byte of sp is always 0x01 in emulation mode.
                if (reg.emulationMode)
                    reg.sp = 0x0100 | Bytes::MaskByte<0>(reg.a);
                else
                    reg.sp = reg.a;
            }
            break;

        case 0x7B: // TDC/TDA - Transfer D to A.
            {
                LogInstruction("%02X: TDC", opcode);
                LoadRegister16Bit(&reg.a, reg.d);
            }
            break;

        case 0x3B: // TSC/TSA - Transfer SP to A.
            {
                LogInstruction("%02X: TSC", opcode);
                LoadRegister16Bit(&reg.a, reg.sp);
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Load opcodes                                                                                                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0xA1: // LDA (Direct,X)
        case 0xA3: // LDA Stack,S
        case 0xA5: // LDA Direct
        case 0xA7: // LDA [Direct]
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
                    LoadRegister16Bit(&reg.a, mode->Read16Bit());
                else
                    LoadRegister8Bit(&reg.a, mode->Read8Bit());
            }
            break;

        case 0xA9: // LDA Immediate
            {
                if (IsAccumulator16Bit())
                    LoadRegister16Bit(&reg.a, ReadPC16Bit());
                else
                    LoadRegister8Bit(&reg.a, ReadPC8Bit());
            }
            break;

        case 0xA2: // LDX Immediate
            {
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.x, ReadPC16Bit());
                else
                    LoadRegister8Bit(&reg.x, ReadPC8Bit());
            }
            break;

        case 0xA6: // LDX Direct
        case 0xAE: // LDX Absolute
        case 0xB6: // LDX Direct,Y
        case 0xBE: // LDX Absolute,Y
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.x, mode->Read16Bit());
                else
                    LoadRegister8Bit(&reg.x, mode->Read8Bit());
            }
            break;

        case 0xA0: // LDY Immediate
            {
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.y, ReadPC16Bit());
                else
                    LoadRegister8Bit(&reg.y, ReadPC8Bit());
            }
            break;

        case 0xA4: // LDY Direct
        case 0xAC: // LDY Absolute
        case 0xB4: // LDY Direct,X
        case 0xBC: // LDY Absolute,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsIndex16Bit())
                    LoadRegister16Bit(&reg.y, mode->Read16Bit());
                else
                    LoadRegister8Bit(&reg.y, mode->Read8Bit());
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Store opcodes                                                                                                     //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                    mode->Write8Bit(reg.a);
            }
            break;

        case 0x86: // STX Direct
        case 0x8E: // STX Absolute
        case 0x96: // STX Direct,Y
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsIndex16Bit())
                    mode->Write16Bit(reg.x);
                else
                    mode->Write8Bit(reg.x);
            }
            break;

        case 0x84: // STY Direct
        case 0x8C: // STY Absolute
        case 0x94: // STY Direct,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsIndex16Bit())
                    mode->Write16Bit(reg.y);
                else
                    mode->Write8Bit(reg.y);
            }
            break;

        case 0x64: // STZ Direct
        case 0x74: // STZ Direct,X
            {
                AddressModePtr &mode = addressModes[opcode & 0x1F];
                mode->LoadAddress();
                if (IsAccumulator16Bit())
                    mode->Write16Bit(0);
                else
                    mode->Write8Bit(0);
            }
            break;

        case 0x9C: // STZ Absolute
            {
                // This opcode doesn't follow the pattern for address mode, so we can't use the addressModes lookup table.
                AddressModeAbsolute mode(this, memory);
                mode.LoadAddress();
                if (IsAccumulator16Bit())
                    mode.Write16Bit(0);
                else
                    mode.Write8Bit(0);
            }
            break;

        case 0x9E: // STZ Absolute,X
            {
                // This opcode doesn't follow the pattern for address mode, so we can't use the addressModes lookup table.
                AddressModeAbsoluteIndexedX mode(this, memory);
                mode.LoadAddress();
                if (IsAccumulator16Bit())
                    mode.Write16Bit(0);
                else
                    mode.Write8Bit(0);
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
                    Push8Bit(Bytes::GetByte<0>(reg.a));
            }
            break;

        case 0xDA: // PHX - Push X
            {
                LogInstruction("%02X: PHX", opcode);
                if (IsIndex16Bit())
                    Push16Bit(reg.x);
                else
                    Push8Bit(Bytes::GetByte<0>(reg.x));
            }
            break;

        case 0x5A: // PHY - Push Y
            {
                LogInstruction("%02X: PHX", opcode);
                if (IsIndex16Bit())
                    Push16Bit(reg.y);
                else
                    Push8Bit(Bytes::GetByte<0>(reg.y));
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
                int8_t value = static_cast<int8_t>(ReadPC8Bit());
                LogInstruction("%02X %02X: PER", opcode, value);
                Push16Bit(reg.pc + value);
            }
            break;

        case 0x68: // PLA - Pull/Pop A
            {
                LogInstruction("%02X: PLA", opcode);
                if (IsAccumulator16Bit())
                {
                    reg.a = Pop16Bit();
                    SetNFlag16Bit(reg.a);
                    SetZFlag16Bit(reg.a);
                }
                else
                {
                    uint8_t value = Pop8Bit();
                    reg.a = Bytes::MaskByte<1>(reg.a) | value;
                    SetNFlag8Bit(value);
                    SetZFlag8Bit(value);
                }
            }
            break;

        case 0xFA: // PLX - Pull/Pop X
            {
                LogInstruction("%02X: PLX", opcode);
                if (IsIndex16Bit())
                {
                    reg.x = Pop16Bit();
                    SetNFlag16Bit(reg.x);
                    SetZFlag16Bit(reg.x);
                }
                else
                {
                    uint8_t value = Pop8Bit();
                    reg.x = Bytes::MaskByte<1>(reg.x) | value;
                    SetNFlag8Bit(value);
                    SetZFlag8Bit(value);
                }
            }
            break;

        case 0x7A: // PLY - Pull/Pop Y
            {
                LogInstruction("%02X: PLY", opcode);
                if (IsIndex16Bit())
                {
                    reg.y = Pop16Bit();
                    SetNFlag16Bit(reg.y);
                    SetZFlag16Bit(reg.y);
                }
                else
                {
                    uint8_t value = Pop8Bit();
                    reg.y = Bytes::MaskByte<1>(reg.y) | value;
                    SetNFlag8Bit(value);
                    SetZFlag8Bit(value);
                }
            }
            break;

        case 0xAB: // PLB - Pull/Pop DB
            {
                LogInstruction("%02X: PLB", opcode);
                reg.db = Pop8Bit();
                SetNFlag8Bit(reg.db);
                SetZFlag8Bit(reg.db);
            }
            break;

        case 0x2B: // PLD - Pull/Pop D
            {
                LogInstruction("%02X: PLD", opcode);
                reg.d = Pop16Bit();
                SetNFlag16Bit(reg.d);
                SetZFlag16Bit(reg.d);
            }
            break;

        case 0x28: // PLP - Pull/Pop P
            {
                LogInstruction("%02X: PLP", opcode);
                reg.p = Pop8Bit();
                // TODO: Clear high byte of reg.x and reg.y if reg.flags.x is 1.
            }
            break;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
// Logical opcodes                                                                                                   //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 0x21: // AND (Direct,X)
        case 0x23: // AND Stack,S
        case 0x25: // AND Direct
        case 0x27: // AND [Direct]
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
                    SetNFlag16Bit(reg.a);
                    SetZFlag16Bit(reg.a);
                }
                else
                {
                    uint8_t value = mode->Read8Bit();
                    reg.a = Bytes::MaskByte<1>(reg.a) | (Bytes::MaskByte<0>(reg.a) & value);
                    SetNFlag8Bit(reg.a);
                    SetZFlag8Bit(reg.a);
                }
            }
            break;

        case 0x29: // AND Immediate
            {
                if (IsAccumulator16Bit())
                {
                    uint16_t value = ReadPC16Bit();
                    reg.a &= value;
                    SetNFlag16Bit(reg.a);
                    SetZFlag16Bit(reg.a);
                }
                else
                {
                    uint8_t value = ReadPC8Bit();
                    reg.a = Bytes::MaskByte<1>(reg.a) | (Bytes::MaskByte<0>(reg.a) & value);
                    SetNFlag8Bit(reg.a);
                    SetZFlag8Bit(reg.a);
                }
            }
            break;

        case 0x00: NotYetImplemented(0x00); break;
        case 0x01: NotYetImplemented(0x01); break;
        case 0x02: NotYetImplemented(0x02); break;
        case 0x03: NotYetImplemented(0x03); break;
        case 0x04: NotYetImplemented(0x04); break;
        case 0x05: NotYetImplemented(0x05); break;
        case 0x06: NotYetImplemented(0x06); break;
        case 0x07: NotYetImplemented(0x07); break;
        //case 0x08: NotYetImplemented(0x08); break;
        case 0x09: NotYetImplemented(0x09); break;
        case 0x0A: NotYetImplemented(0x0A); break;
        //case 0x0B: NotYetImplemented(0x0B); break;
        case 0x0C: NotYetImplemented(0x0C); break;
        case 0x0D: NotYetImplemented(0x0D); break;
        case 0x0E: NotYetImplemented(0x0E); break;
        case 0x0F: NotYetImplemented(0x0F); break;

        case 0x10: NotYetImplemented(0x10); break;
        case 0x11: NotYetImplemented(0x11); break;
        case 0x12: NotYetImplemented(0x12); break;
        case 0x13: NotYetImplemented(0x13); break;
        case 0x14: NotYetImplemented(0x14); break;
        case 0x15: NotYetImplemented(0x15); break;
        case 0x16: NotYetImplemented(0x16); break;
        case 0x17: NotYetImplemented(0x17); break;
        case 0x18: NotYetImplemented(0x18); break;
        case 0x19: NotYetImplemented(0x19); break;
        case 0x1A: NotYetImplemented(0x1A); break;
        //case 0x1B: NotYetImplemented(0x1B); break;
        case 0x1C: NotYetImplemented(0x1C); break;
        case 0x1D: NotYetImplemented(0x1D); break;
        case 0x1E: NotYetImplemented(0x1E); break;
        case 0x1F: NotYetImplemented(0x1F); break;

        case 0x20: NotYetImplemented(0x20); break;
        //case 0x21: NotYetImplemented(0x21); break;
        case 0x22: NotYetImplemented(0x22); break;
        //case 0x23: NotYetImplemented(0x23); break;
        case 0x24: NotYetImplemented(0x24); break;
        //case 0x25: NotYetImplemented(0x25); break;
        case 0x26: NotYetImplemented(0x26); break;
        //case 0x27: NotYetImplemented(0x27); break;
        //case 0x28: NotYetImplemented(0x28); break;
        //case 0x29: NotYetImplemented(0x29); break;
        case 0x2A: NotYetImplemented(0x2A); break;
        //case 0x2B: NotYetImplemented(0x2B); break;
        case 0x2C: NotYetImplemented(0x2C); break;
        //case 0x2D: NotYetImplemented(0x2D); break;
        case 0x2E: NotYetImplemented(0x2E); break;
        //case 0x2F: NotYetImplemented(0x2F); break;

        case 0x30: NotYetImplemented(0x30); break;
        //case 0x31: NotYetImplemented(0x31); break;
        //case 0x32: NotYetImplemented(0x32); break;
        //case 0x33: NotYetImplemented(0x33); break;
        case 0x34: NotYetImplemented(0x34); break;
        //case 0x35: NotYetImplemented(0x35); break;
        case 0x36: NotYetImplemented(0x36); break;
        //case 0x37: NotYetImplemented(0x37); break;
        case 0x38: NotYetImplemented(0x38); break;
        //case 0x39: NotYetImplemented(0x39); break;
        case 0x3A: NotYetImplemented(0x3A); break;
        //case 0x3B: NotYetImplemented(0x3B); break;
        case 0x3C: NotYetImplemented(0x3C); break;
        //case 0x3D: NotYetImplemented(0x3D); break;
        case 0x3E: NotYetImplemented(0x3E); break;
        //case 0x3F: NotYetImplemented(0x3F); break;

        case 0x40: NotYetImplemented(0x40); break;
        case 0x41: NotYetImplemented(0x41); break;
        case 0x42: NotYetImplemented(0x42); break;
        case 0x43: NotYetImplemented(0x43); break;
        case 0x44: NotYetImplemented(0x44); break;
        case 0x45: NotYetImplemented(0x45); break;
        case 0x46: NotYetImplemented(0x46); break;
        case 0x47: NotYetImplemented(0x47); break;
        //case 0x48: NotYetImplemented(0x48); break;
        case 0x49: NotYetImplemented(0x49); break;
        case 0x4A: NotYetImplemented(0x4A); break;
        //case 0x4B: NotYetImplemented(0x4B); break;
        case 0x4C: NotYetImplemented(0x4C); break;
        case 0x4D: NotYetImplemented(0x4D); break;
        case 0x4E: NotYetImplemented(0x4E); break;
        case 0x4F: NotYetImplemented(0x4F); break;

        case 0x50: NotYetImplemented(0x50); break;
        case 0x51: NotYetImplemented(0x51); break;
        case 0x52: NotYetImplemented(0x52); break;
        case 0x53: NotYetImplemented(0x53); break;
        case 0x54: NotYetImplemented(0x54); break;
        case 0x55: NotYetImplemented(0x55); break;
        case 0x56: NotYetImplemented(0x56); break;
        case 0x57: NotYetImplemented(0x57); break;
        case 0x58: NotYetImplemented(0x58); break;
        case 0x59: NotYetImplemented(0x59); break;
        //case 0x5A: NotYetImplemented(0x5A); break;
        //case 0x5B: NotYetImplemented(0x5B); break;
        case 0x5C: NotYetImplemented(0x5C); break;
        case 0x5D: NotYetImplemented(0x5D); break;
        case 0x5E: NotYetImplemented(0x5E); break;
        case 0x5F: NotYetImplemented(0x5F); break;

        case 0x60: NotYetImplemented(0x60); break;
        case 0x61: NotYetImplemented(0x61); break;
        //case 0x62: NotYetImplemented(0x62); break;
        case 0x63: NotYetImplemented(0x63); break;
        //case 0x64: NotYetImplemented(0x64); break;
        case 0x65: NotYetImplemented(0x65); break;
        case 0x66: NotYetImplemented(0x66); break;
        case 0x67: NotYetImplemented(0x67); break;
        //case 0x68: NotYetImplemented(0x68); break;
        case 0x69: NotYetImplemented(0x69); break;
        case 0x6A: NotYetImplemented(0x6A); break;
        case 0x6B: NotYetImplemented(0x6B); break;
        case 0x6C: NotYetImplemented(0x6C); break;
        case 0x6D: NotYetImplemented(0x6D); break;
        case 0x6E: NotYetImplemented(0x6E); break;
        case 0x6F: NotYetImplemented(0x6F); break;

        case 0x70: NotYetImplemented(0x70); break;
        case 0x71: NotYetImplemented(0x71); break;
        case 0x72: NotYetImplemented(0x72); break;
        case 0x73: NotYetImplemented(0x73); break;
        //case 0x74: NotYetImplemented(0x74); break;
        case 0x75: NotYetImplemented(0x75); break;
        case 0x76: NotYetImplemented(0x76); break;
        case 0x77: NotYetImplemented(0x77); break;
        case 0x78: NotYetImplemented(0x78); break;
        case 0x79: NotYetImplemented(0x79); break;
        //case 0x7A: NotYetImplemented(0x7A); break;
        //case 0x7B: NotYetImplemented(0x7B); break;
        case 0x7C: NotYetImplemented(0x7C); break;
        case 0x7D: NotYetImplemented(0x7D); break;
        case 0x7E: NotYetImplemented(0x7E); break;
        case 0x7F: NotYetImplemented(0x7F); break;

        case 0x80: NotYetImplemented(0x80); break;
        //case 0x81: NotYetImplemented(0x81); break;
        case 0x82: NotYetImplemented(0x82); break;
        //case 0x83: NotYetImplemented(0x83); break;
        //case 0x84: NotYetImplemented(0x84); break;
        //case 0x85: NotYetImplemented(0x85); break;
        //case 0x86: NotYetImplemented(0x86); break;
        //case 0x87: NotYetImplemented(0x87); break;
        case 0x88: NotYetImplemented(0x88); break;
        case 0x89: NotYetImplemented(0x89); break;
        //case 0x8A: NotYetImplemented(0x8A); break;
        //case 0x8B: NotYetImplemented(0x8B); break;
        //case 0x8C: NotYetImplemented(0x8C); break;
        //case 0x8D: NotYetImplemented(0x8D); break;
        //case 0x8E: NotYetImplemented(0x8E); break;
        //case 0x8F: NotYetImplemented(0x8F); break;

        case 0x90: NotYetImplemented(0x90); break;
        //case 0x91: NotYetImplemented(0x91); break;
        //case 0x92: NotYetImplemented(0x92); break;
        case 0x93: NotYetImplemented(0x93); break;
        //case 0x94: NotYetImplemented(0x94); break;
        //case 0x95: NotYetImplemented(0x95); break;
        //case 0x96: NotYetImplemented(0x96); break;
        //case 0x97: NotYetImplemented(0x97); break;
        //case 0x98: NotYetImplemented(0x98); break;
        //case 0x99: NotYetImplemented(0x99); break;
        //case 0x9A: NotYetImplemented(0x9A); break;
        //case 0x9B: NotYetImplemented(0x9B); break;
        //case 0x9C: NotYetImplemented(0x9C); break;
        //case 0x9D: NotYetImplemented(0x9D); break;
        //case 0x9E: NotYetImplemented(0x9E); break;
        //case 0x9F: NotYetImplemented(0x9F); break;

        //case 0xA0: NotYetImplemented(0xA0); break;
        //case 0xA1: NotYetImplemented(0xA1); break;
        //case 0xA2: NotYetImplemented(0xA2); break;
        //case 0xA3: NotYetImplemented(0xA3); break;
        //case 0xA4: NotYetImplemented(0xA4); break;
        //case 0xA5: NotYetImplemented(0xA5); break;
        //case 0xA6: NotYetImplemented(0xA6); break;
        //case 0xA7: NotYetImplemented(0xA7); break;
        //case 0xA8: NotYetImplemented(0xA8); break;
        //case 0xA9: NotYetImplemented(0xA9); break;
        //case 0xAA: NotYetImplemented(0xAA); break;
        //case 0xAB: NotYetImplemented(0xAB); break;
        //case 0xAC: NotYetImplemented(0xAC); break;
        //case 0xAD: NotYetImplemented(0xAD); break;
        //case 0xAE: NotYetImplemented(0xAE); break;
        //case 0xAF: NotYetImplemented(0xAF); break;

        case 0xB0: NotYetImplemented(0xB0); break;
        //case 0xB1: NotYetImplemented(0xB1); break;
        //case 0xB2: NotYetImplemented(0xB2); break;
        //case 0xB3: NotYetImplemented(0xB3); break;
        //case 0xB4: NotYetImplemented(0xB4); break;
        //case 0xB5: NotYetImplemented(0xB5); break;
        //case 0xB6: NotYetImplemented(0xB6); break;
        //case 0xB7: NotYetImplemented(0xB7); break;
        case 0xB8: NotYetImplemented(0xB8); break;
        //case 0xB9: NotYetImplemented(0xB9); break;
        //case 0xBA: NotYetImplemented(0xBA); break;
        //case 0xBB: NotYetImplemented(0xBB); break;
        //case 0xBC: NotYetImplemented(0xBC); break;
        //case 0xBD: NotYetImplemented(0xBD); break;
        //case 0xBE: NotYetImplemented(0xBE); break;
        //case 0xBF: NotYetImplemented(0xBF); break;

        case 0xC0: NotYetImplemented(0xC0); break;
        case 0xC1: NotYetImplemented(0xC1); break;
        case 0xC2: NotYetImplemented(0xC2); break;
        case 0xC3: NotYetImplemented(0xC3); break;
        case 0xC4: NotYetImplemented(0xC4); break;
        case 0xC5: NotYetImplemented(0xC5); break;
        case 0xC6: NotYetImplemented(0xC6); break;
        case 0xC7: NotYetImplemented(0xC7); break;
        case 0xC8: NotYetImplemented(0xC8); break;
        case 0xC9: NotYetImplemented(0xC9); break;
        case 0xCA: NotYetImplemented(0xCA); break;
        case 0xCB: NotYetImplemented(0xCB); break;
        case 0xCC: NotYetImplemented(0xCC); break;
        case 0xCD: NotYetImplemented(0xCD); break;
        case 0xCE: NotYetImplemented(0xCE); break;
        case 0xCF: NotYetImplemented(0xCF); break;

        case 0xD0: NotYetImplemented(0xD0); break;
        case 0xD1: NotYetImplemented(0xD1); break;
        case 0xD2: NotYetImplemented(0xD2); break;
        case 0xD3: NotYetImplemented(0xD3); break;
        //case 0xD4: NotYetImplemented(0xD4); break;
        case 0xD5: NotYetImplemented(0xD5); break;
        case 0xD6: NotYetImplemented(0xD6); break;
        case 0xD7: NotYetImplemented(0xD7); break;
        case 0xD8: NotYetImplemented(0xD8); break;
        case 0xD9: NotYetImplemented(0xD9); break;
        //case 0xDA: NotYetImplemented(0xDA); break;
        case 0xDB: NotYetImplemented(0xDB); break;
        case 0xDC: NotYetImplemented(0xDC); break;
        case 0xDD: NotYetImplemented(0xDD); break;
        case 0xDE: NotYetImplemented(0xDE); break;
        case 0xDF: NotYetImplemented(0xDF); break;

        case 0xE0: NotYetImplemented(0xE0); break;
        case 0xE1: NotYetImplemented(0xE1); break;
        case 0xE2: NotYetImplemented(0xE2); break;
        case 0xE3: NotYetImplemented(0xE3); break;
        case 0xE4: NotYetImplemented(0xE4); break;
        case 0xE5: NotYetImplemented(0xE5); break;
        case 0xE6: NotYetImplemented(0xE6); break;
        case 0xE7: NotYetImplemented(0xE7); break;
        case 0xE8: NotYetImplemented(0xE8); break;
        case 0xE9: NotYetImplemented(0xE9); break;
        case 0xEA: NotYetImplemented(0xEA); break;
        case 0xEB: NotYetImplemented(0xEB); break;
        case 0xEC: NotYetImplemented(0xEC); break;
        case 0xED: NotYetImplemented(0xED); break;
        case 0xEE: NotYetImplemented(0xEE); break;
        case 0xEF: NotYetImplemented(0xEF); break;

        case 0xF0: NotYetImplemented(0xF0); break;
        case 0xF1: NotYetImplemented(0xF1); break;
        case 0xF2: NotYetImplemented(0xF2); break;
        case 0xF3: NotYetImplemented(0xF3); break;
        //case 0xF4: NotYetImplemented(0xF4); break;
        case 0xF5: NotYetImplemented(0xF5); break;
        case 0xF6: NotYetImplemented(0xF6); break;
        case 0xF7: NotYetImplemented(0xF7); break;
        case 0xF8: NotYetImplemented(0xF8); break;
        case 0xF9: NotYetImplemented(0xF9); break;
        //case 0xFA: NotYetImplemented(0xFA); break;
        case 0xFB: NotYetImplemented(0xFB); break;
        case 0xFC: NotYetImplemented(0xFC); break;
        case 0xFD: NotYetImplemented(0xFD); break;
        case 0xFE: NotYetImplemented(0xFE); break;
        case 0xFF: NotYetImplemented(0xFF); break;
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