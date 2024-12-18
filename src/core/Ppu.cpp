#include "IoRegisters.h"
#include "DebuggerInterface.h"
#include "Memory.h"
#include "Ppu.h"


Ppu::Ppu(Memory *memory, TimerSubject *timerSubject, DisplayInterface *displayInterface, DebuggerInterface *debuggerInterface) :
    memory(memory),
    oam{0},
    vram{0},
    cgram{0},
    palette{0},
    frameBuffer{0},
    debuggerInterface(debuggerInterface),
    displayInterface(displayInterface),
    isHBlank(true),
    isVBlank(false),
    clockCounter(0),
    scanline(0),
    isForcedBlank(false),
    brightness(0),
    bgMode(0),
    bgOffsetLatch(0),
    bgHOffsetLatch(0),
    bgHOffset{0, 0, 0, 0},
    bgVOffset{0, 0, 0, 0},
    cgramRwAddr(0),
    cgramLatch(0),
    vramIncrement(0),
    isVramIncrementOnHigh(false),
    vramRwAddr(0),
    vramPrefetch{0,0},
    oamRwAddr(0),
    oamLatch(0),
    regINIDISP(memory->AttachIoRegister(eRegINIDISP, this)),
    regOBSEL(memory->AttachIoRegister(eRegOBSEL, this)),
    regOAMADDL(memory->AttachIoRegister(eRegOAMADDL, this)),
    regOAMADDH(memory->AttachIoRegister(eRegOAMADDH, this)),
    regOAMDATA(memory->AttachIoRegister(eRegOAMDATA, this)),
    regBGMODE(memory->AttachIoRegister(eRegBGMODE, this)),
    regMOSAIC(memory->AttachIoRegister(eRegMOSAIC, this)),
    regBG1SC(memory->AttachIoRegister(eRegBG1SC, this)),
    regBG2SC(memory->AttachIoRegister(eRegBG2SC, this)),
    regBG3SC(memory->AttachIoRegister(eRegBG3SC, this)),
    regBG4SC(memory->AttachIoRegister(eRegBG4SC, this)),
    regBG12NBA(memory->AttachIoRegister(eRegBG12NBA, this)),
    regBG34NBA(memory->AttachIoRegister(eRegBG34NBA, this)),
    regBG1HOFS(memory->AttachIoRegister(eRegBG1HOFS, this)),
    regBG1VOFS(memory->AttachIoRegister(eRegBG1VOFS, this)),
    regBG2HOFS(memory->AttachIoRegister(eRegBG2HOFS, this)),
    regBG2VOFS(memory->AttachIoRegister(eRegBG2VOFS, this)),
    regBG3HOFS(memory->AttachIoRegister(eRegBG3HOFS, this)),
    regBG3VOFS(memory->AttachIoRegister(eRegBG3VOFS, this)),
    regBG4HOFS(memory->AttachIoRegister(eRegBG4HOFS, this)),
    regBG4VOFS(memory->AttachIoRegister(eRegBG4VOFS, this)),
    regVMAIN(memory->AttachIoRegister(eRegVMAIN, this)),
    regVMADDL(memory->AttachIoRegister(eRegVMADDL, this)),
    regVMADDH(memory->AttachIoRegister(eRegVMADDH, this)),
    regVMDATAL(memory->AttachIoRegister(eRegVMDATAL, this)),
    regVMDATAH(memory->AttachIoRegister(eRegVMDATAH, this)),
    regM7SEL(memory->AttachIoRegister(eRegM7SEL, this)),
    regM7A(memory->AttachIoRegister(eRegM7A, this)),
    regM7B(memory->AttachIoRegister(eRegM7B, this)),
    regM7C(memory->AttachIoRegister(eRegM7C, this)),
    regM7D(memory->AttachIoRegister(eRegM7D, this)),
    regM7X(memory->AttachIoRegister(eRegM7X, this)),
    regM7Y(memory->AttachIoRegister(eRegM7Y, this)),
    regCGADD(memory->AttachIoRegister(eRegCGADD, this)),
    regCGDATA(memory->AttachIoRegister(eRegCGDATA, this)),
    regW12SEL(memory->AttachIoRegister(eRegW12SEL, this)),
    regW34SEL(memory->AttachIoRegister(eRegW34SEL, this)),
    regWOBJSEL(memory->AttachIoRegister(eRegWOBJSEL, this)),
    regWH0(memory->AttachIoRegister(eRegWH0, this)),
    regWH1(memory->AttachIoRegister(eRegWH1, this)),
    regWH2(memory->AttachIoRegister(eRegWH2, this)),
    regWH3(memory->AttachIoRegister(eRegWH3, this)),
    regWBGLOG(memory->AttachIoRegister(eRegWBGLOG, this)),
    regWOBJLOG(memory->AttachIoRegister(eRegWOBJLOG, this)),
    regTM(memory->AttachIoRegister(eRegTM, this)),
    regTS(memory->AttachIoRegister(eRegTS, this)),
    regTMW(memory->AttachIoRegister(eRegTMW, this)),
    regTSW(memory->AttachIoRegister(eRegTSW, this)),
    regCGWSEL(memory->AttachIoRegister(eRegCGWSEL, this)),
    regCGADSUB(memory->AttachIoRegister(eRegCGADSUB, this)),
    regCOLDATA(memory->AttachIoRegister(eRegCOLDATA, this)),
    regSETINI(memory->AttachIoRegister(eRegSETINI, this)),
    regMPYL(memory->AttachIoRegister(eRegMPYL, this)),
    regMPYM(memory->AttachIoRegister(eRegMPYM, this)),
    regMPYH(memory->AttachIoRegister(eRegMPYH, this)),
    regSLHV(memory->AttachIoRegister(eRegSLHV, this)),
    regRDOAM(memory->AttachIoRegister(eRegRDOAM, this)),
    regRDVRAML(memory->AttachIoRegister(eRegRDVRAML, this)),
    regRDVRAMH(memory->AttachIoRegister(eRegRDVRAMH, this)),
    regRDCGRAM(memory->AttachIoRegister(eRegRDCGRAM, this)),
    regOPHCT(memory->AttachIoRegister(eRegOPHCT, this)),
    regOPVCT(memory->AttachIoRegister(eRegOPVCT, this)),
    regSTAT77(memory->AttachIoRegister(eRegSTAT77, this)),
    regSTAT78(memory->AttachIoRegister(eRegSTAT78, this))
{
    timerSubject->AttachObserver(this);
}


uint8_t Ppu::ReadRegister(EIORegisters ioReg) const
{
    LogPpu("Ppu::ReadRegister %04X", ioReg);

    switch (ioReg)
    {
        case eRegINIDISP:
            return *regINIDISP;
        case eRegOBSEL:
            return *regOBSEL;
        case eRegOAMADDL:
            return *regOAMADDL;
        case eRegOAMADDH:
            return *regOAMADDH;
        case eRegOAMDATA:
            return *regOAMDATA;
        case eRegBGMODE:
            return *regBGMODE;
        case eRegMOSAIC:
            return *regMOSAIC;
        case eRegBG1SC:
            return *regBG1SC;
        case eRegBG2SC:
            return *regBG2SC;
        case eRegBG3SC:
            return *regBG3SC;
        case eRegBG4SC:
            return *regBG4SC;
        case eRegBG12NBA:
            return *regBG12NBA;
        case eRegBG34NBA:
            return *regBG34NBA;
        case eRegBG1HOFS:
            return *regBG1HOFS;
        case eRegBG1VOFS:
            return *regBG1VOFS;
        case eRegBG2HOFS:
            return *regBG2HOFS;
        case eRegBG2VOFS:
            return *regBG2VOFS;
        case eRegBG3HOFS:
            return *regBG3HOFS;
        case eRegBG3VOFS:
            return *regBG3VOFS;
        case eRegBG4HOFS:
            return *regBG4HOFS;
        case eRegBG4VOFS:
            return *regBG4VOFS;
        case eRegVMAIN:
            return *regVMAIN;
        case eRegVMADDL:
            return *regVMADDL;
        case eRegVMADDH:
            return *regVMADDH;
        case eRegVMDATAL:
            return *regVMDATAL;
        case eRegVMDATAH:
            return *regVMDATAH;
        case eRegM7SEL:
            return *regM7SEL;
        case eRegM7A:
            return *regM7A;
        case eRegM7B:
            return *regM7B;
        case eRegM7C:
            return *regM7C;
        case eRegM7D:
            return *regM7D;
        case eRegM7X:
            return *regM7X;
        case eRegM7Y:
            return *regM7Y;
        case eRegCGADD:
            return *regCGADD;
        case eRegCGDATA:
            return *regCGDATA;
        case eRegW12SEL:
            return *regW12SEL;
        case eRegW34SEL:
            return *regW34SEL;
        case eRegWOBJSEL:
            return *regWOBJSEL;
        case eRegWH0:
            return *regWH0;
        case eRegWH1:
            return *regWH1;
        case eRegWH2:
            return *regWH2;
        case eRegWH3:
            return *regWH3;
        case eRegWBGLOG:
            return *regWBGLOG;
        case eRegWOBJLOG:
            return *regWOBJLOG;
        case eRegTM:
            return *regTM;
        case eRegTS:
            return *regTS;
        case eRegTMW:
            return *regTMW;
        case eRegTSW:
            return *regTSW;
        case eRegCGWSEL:
            return *regCGWSEL;
        case eRegCGADSUB:
            return *regCGADSUB;
        case eRegCOLDATA:
            return *regCOLDATA;
        case eRegSETINI:
            return *regSETINI;
        case eRegMPYL:
            return *regMPYL;
        case eRegMPYM:
            return *regMPYM;
        case eRegMPYH:
            return *regMPYH;
        case eRegSLHV:
            return *regSLHV;
        case eRegRDOAM:
            return *regRDOAM;
        case eRegRDVRAML:
            return *regRDVRAML;
        case eRegRDVRAMH:
            return *regRDVRAMH;
        case eRegRDCGRAM:
            return *regRDCGRAM;
        case eRegOPHCT:
            return *regOPHCT;
        case eRegOPVCT:
            return *regOPVCT;
        case eRegSTAT77:
            return *regSTAT77;
        case eRegSTAT78:
            return *regSTAT78;
        default:
            throw std::range_error(fmt("Ppu doesnt handle reads to 0x%04X", ioReg));
    }
    return 0;
}


bool Ppu::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogPpu("Ppu::WriteRegister %04X, %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegINIDISP: // 0x2100
            *regINIDISP = byte;
            isForcedBlank = byte & 0x80;
            brightness = byte & 0x0F;
            // TODO: reset oamRwAddr if this is written on the first scanline of vblank (225/240).
            LogPpu("ForcedBlank=%d Brightness=%d", isForcedBlank, brightness);
            return true;
        case eRegOBSEL: // 0x2101
            *regOBSEL = byte;
            LogPpu("OBSEL=%02X. NYI", byte);
            return true;
        case eRegOAMADDL: // 0x2102
            *regOAMADDL = byte;
            // This is a word address, so left shift 1 to get the byte address.
            oamRwAddr = Bytes::Make16Bit((*regOAMADDH) & 0x01, byte) << 1;
            LogPpu("oamRwAddr=%04X", oamRwAddr);
            return true;
        case eRegOAMADDH: // 0x2103
            *regOAMADDH = byte;
            // This is a word address, so left shift 1 to get the byte address.
            oamRwAddr = Bytes::Make16Bit((byte) & 0x01, *regOAMADDL) << 1;
            LogPpu("oamRwAddr=%04X", oamRwAddr);
            return true;
        case eRegOAMDATA: // 0x2104
            *regOAMDATA = byte;

            // The first write gets saved, and both bytes get written to oam on the second write to the register.
            if ((oamRwAddr & 0x01) == 0)
            {
                oamLatch = byte;
                LogPpu("Saving oam byte %02X", byte);
            }

            // Writes to the high 32 bytes get applied immediately.
            if (oamRwAddr >= 0x200)
            {
                // Only the last 5 bits count. Anything higher than 0x21F is mirrored.
                oam[0x200 | (oamRwAddr & 0x1F)] = byte;
                LogPpu("Writing byte to high oam %04X(%04X)=%02X", oamRwAddr, (0x200 | (oamRwAddr & 0x1F)), byte);
            }
            else if (oamRwAddr & 0x01)
            {
                oam[oamRwAddr - 1] = oamLatch;
                oam[oamRwAddr] = byte;
                LogPpu("Writing word to oam %04X=%02X%02X", oamRwAddr, byte, oamLatch);
            }
            // 0-0x21F are valid, anything above is mirrored.
            oamRwAddr = (oamRwAddr + 1) & 0x3FF;
            return true;
        case eRegBGMODE: // 0x2105
            *regBGMODE = byte;
            bgMode = byte & 0x07;
            LogPpu("bgMode=%d", bgMode);
            return true;
        case eRegMOSAIC: // 0x2106
            *regMOSAIC = byte;
            LogPpu("MosaicSize=%d MosaicLayers=%d,%d,%d,%d", (byte >> 4), Bytes::GetBit<0>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<3>(byte));
            return true;
        case eRegBG1SC: // 0x2107
            *regBG1SC = byte;
            LogPpu("Screen1 Address=%04X Size=%dx%d", (byte >> 2), (32 << Bytes::GetBit<0>(byte)), (32 << Bytes::GetBit<1>(byte)));
            return true;
        case eRegBG2SC: // 0x2108
            *regBG2SC = byte;
            LogPpu("Screen2 Address=%04X Size=%dx%d", (byte >> 2), (32 << Bytes::GetBit<0>(byte)), (32 << Bytes::GetBit<1>(byte)));
            return true;
        case eRegBG3SC: // 0x2109
            *regBG3SC = byte;
            LogPpu("Screen3 Address=%04X Size=%dx%d", (byte >> 2), (32 << Bytes::GetBit<0>(byte)), (32 << Bytes::GetBit<1>(byte)));
            return true;
        case eRegBG4SC: // 0x210A
            *regBG4SC = byte;
            LogPpu("Screen4 Address=%04X Size=%dx%d", (byte >> 2), (32 << Bytes::GetBit<0>(byte)), (32 << Bytes::GetBit<1>(byte)));
            return true;
        case eRegBG12NBA: // 0x210B
            *regBG12NBA = byte;
            LogPpu("Tile Address BG1=%02x BG2=%02X", byte & 0x0F, byte >> 4);
            return true;
        case eRegBG34NBA: // 0x210C
            *regBG34NBA = byte;
            LogPpu("Tile Address BG3=%02x BG4=%02X", byte & 0x0F, byte >> 4);
            return true;
        case eRegBG1HOFS: // 0x210D
            *regBG1HOFS = byte;
            LogPpu("bgOffsetLatch=%02X bgHOffsetLatch=%02X", bgOffsetLatch, bgHOffsetLatch);
            // This is a write twice register.
            bgHOffset[0] = (byte << 8) | (bgOffsetLatch & ~0x07) | (bgHOffsetLatch & 0x07);
            bgOffsetLatch = byte;
            bgHOffsetLatch = byte;
            LogPpu("bgHOffset[0]=%04X", bgHOffset[0]);
            return true;
        case eRegBG1VOFS: // 0x210E
            *regBG1VOFS = byte;
            LogPpu("bgOffsetLatch=%02X", bgOffsetLatch);
            // This is a write twice register.
            bgVOffset[0] = (byte << 8) | bgOffsetLatch;
            bgOffsetLatch = byte;
            LogPpu("bgVOffset[0]=%04X", bgVOffset[0]);
            return true;
        case eRegBG2HOFS: // 0x210F
            *regBG2HOFS = byte;
            LogPpu("bgOffsetLatch=%02X bgHOffsetLatch=%02X", bgOffsetLatch, bgHOffsetLatch);
            // This is a write twice register.
            bgHOffset[1] = (byte << 8) | (bgOffsetLatch & ~0x07) | (bgHOffsetLatch & 0x07);
            bgOffsetLatch = byte;
            bgHOffsetLatch = byte;
            LogPpu("bgHOffset[1]=%04X", bgHOffset[1]);
            return true;
        case eRegBG2VOFS: // 0x2110
            *regBG2VOFS = byte;
            LogPpu("bgOffsetLatch=%02X", bgOffsetLatch);
            // This is a write twice register.
            bgVOffset[1] = (byte << 8) | bgOffsetLatch;
            bgOffsetLatch = byte;
            LogPpu("bgVOffset[1]=%04X", bgVOffset[1]);
            return true;
        case eRegBG3HOFS: // 0x2111
            *regBG3HOFS = byte;
            LogPpu("bgOffsetLatch=%02X bgHOffsetLatch=%02X", bgOffsetLatch, bgHOffsetLatch);
            // This is a write twice register.
            bgHOffset[2] = (byte << 8) | (bgOffsetLatch & ~0x07) | (bgHOffsetLatch & 0x07);
            bgOffsetLatch = byte;
            bgHOffsetLatch = byte;
            LogPpu("bgHOffset[2]=%04X", bgHOffset[2]);
            return true;
        case eRegBG3VOFS: // 0x2112
            *regBG3VOFS = byte;
            LogPpu("bgOffsetLatch=%02X", bgOffsetLatch);
            // This is a write twice register.
            bgVOffset[2] = (byte << 8) | bgOffsetLatch;
            bgOffsetLatch = byte;
            LogPpu("bgVOffset[2]=%04X", bgVOffset[2]);
            return true;
        case eRegBG4HOFS: // 0x2113
            *regBG4HOFS = byte;
            LogPpu("bgOffsetLatch=%02X bgHOffsetLatch=%02X", bgOffsetLatch, bgHOffsetLatch);
            // This is a write twice register.
            bgHOffset[3] = (byte << 8) | (bgOffsetLatch & ~0x07) | (bgHOffsetLatch & 0x07);
            bgOffsetLatch = byte;
            bgHOffsetLatch = byte;
            LogPpu("bgHOffset[3]=%04X", bgHOffset[3]);
            return true;
        case eRegBG4VOFS: // 0x2114
            *regBG4VOFS = byte;
            LogPpu("bgOffsetLatch=%02X", bgOffsetLatch);
            // This is a write twice register.
            bgVOffset[3] = (byte << 8) | bgOffsetLatch;
            bgOffsetLatch = byte;
            LogPpu("bgVOffset[3]=%04X", bgVOffset[3]);
            return true;
        case eRegVMAIN: // 0x2115
            *regVMAIN = byte;
            if ((byte & 0x0C) != 0)
                throw NotYetImplementedException(fmt("Address translation NYI. Byte=%02X", byte));
            isVramIncrementOnHigh = byte & 0x80;
            switch (byte & 0x03)
            {
                case 0: vramIncrement = 1; break;
                case 1: vramIncrement = 32; break;
                case 2: case 3: vramIncrement = 128; break;
            }
            LogPpu("Increment VRAM by %d after reading %s byte", vramIncrement, isVramIncrementOnHigh ? "High" : "Low");
            return true;
        case eRegVMADDL: // 0x2116
            *regVMADDL = byte;

            // This is a word address, so left shift 1 to get the byte address.
            vramRwAddr = Bytes::Make16Bit(*regVMADDH, byte) << 1;
            LogPpu("vramRwAddr=%04X", vramRwAddr);

            // Prefetch the bytes when the address changes.
            vramPrefetch[0] = vram[vramRwAddr];
            vramPrefetch[1] = vram[vramRwAddr + 1];

            return true;
        case eRegVMADDH: // 0x2117
            *regVMADDH = byte;

            // This is a word address, so left shift 1 to get the byte address.
            vramRwAddr = Bytes::Make16Bit(byte, *regVMADDL) << 1;
            LogPpu("vramRwAddr=%04X", vramRwAddr);

            // Prefetch the bytes when the address changes.
            vramPrefetch[0] = vram[vramRwAddr];
            vramPrefetch[1] = vram[vramRwAddr + 1];

            return true;
        case eRegVMDATAL: // 0x2118
            *regVMDATAL = byte;
            vram[vramRwAddr] = byte;
            LogPpu("Write to vram %04X=%02X", vramRwAddr, byte);
            if (!isVramIncrementOnHigh)
            {
                // This is a word address, so left shift 1 to get the byte address.
                vramRwAddr += vramIncrement << 1;
            }
            return true;
        case eRegVMDATAH: // 0x2119
            *regVMDATAH = byte;
            vram[vramRwAddr + 1] = byte;
            LogPpu("Write to vram %04X=%02X", vramRwAddr, byte);
            if (isVramIncrementOnHigh)
            {
                // This is a word address, so left shift 1 to get the byte address.
                vramRwAddr += vramIncrement << 1;
            }
            return true;
        case eRegM7SEL:
            *regM7SEL = byte;
            return false;
        case eRegM7A: // 0x211B
            *regM7A = byte;
            LogPpu("M7A=%02X. NYI", byte);
            return true;
        case eRegM7B: // 0x211C
            *regM7B = byte;
            LogPpu("M7B=%02X. NYI", byte);
            return true;
        case eRegM7C: // 0x211D
            *regM7C = byte;
            LogPpu("M7C=%02X. NYI", byte);
            return true;
        case eRegM7D: // 0x211E
            *regM7D = byte;
            LogPpu("M7D=%02X. NYI", byte);
            return true;
        case eRegM7X: // 0x211F
            *regM7X = byte;
            LogPpu("M7X=%02X. NYI", byte);
            return true;
        case eRegM7Y: // 0x2120
            *regM7Y = byte;
            LogPpu("M7Y=%02X. NYI", byte);
            return true;
        case eRegCGADD: // 0x2121
            *regCGADD = byte;
            // This is a word address, so left shift 1 to get the byte address.
            // Since the low bit is always 0, this also resets the read/write twice sequence on CGDATA/RDCGRAM.
            cgramRwAddr = byte << 1;
            LogPpu("Palette color index=%04X", cgramRwAddr);
            return true;
        case eRegCGDATA: // 0x2122
            *regCGDATA = byte;
            // The first write gets saved, and both bytes get written to cgram on the second write to the register.
            if ((cgramRwAddr & 0x01) == 0)
            {
                cgramLatch = byte;
                LogPpu("Saving cgram byte %02X", byte);
            }
            else
            {
                cgram[cgramRwAddr - 1] = cgramLatch;
                cgram[cgramRwAddr] = byte;
                LogPpu("Writing word to cgram %04X %02X%02X", cgramRwAddr, byte, cgramLatch);

                // Convert to ARGB and store in palette.
                palette[cgramRwAddr >> 1] = ConvertBGR555toARGB888(Bytes::Make16Bit(byte, cgramLatch));
            }
            cgramRwAddr++;
            return true;
        case eRegW12SEL: // 0x2123
            *regW12SEL = byte;
            LogPpu("W12SEL=%02X. NYI", byte);
            return true;
        case eRegW34SEL: // 0x2124
            *regW34SEL = byte;
            LogPpu("W34SEL=%02X. NYI", byte);
            return true;
        case eRegWOBJSEL: // 0x2125
            *regWOBJSEL = byte;
            LogPpu("OBJSEL=%02X. NYI", byte);
            return true;
        case eRegWH0: // 0x2126
            *regWH0 = byte;
            LogPpu("WH0=%02X. NYI", byte);
            return true;
        case eRegWH1: // 0x2127
            *regWH1 = byte;
            LogPpu("WH1=%02X. NYI", byte);
            return true;
        case eRegWH2: // 0x2128
            *regWH2 = byte;
            LogPpu("WH2=%02X. NYI", byte);
            return true;
        case eRegWH3: // 0x2129
            *regWH3 = byte;
            LogPpu("WH3=%02X. NYI", byte);
            return true;
        case eRegWBGLOG: // 0x212A
            *regWBGLOG = byte;
            LogPpu("WBGLOG=%02X. NYI", byte);
            return true;
        case eRegWOBJLOG: // 0x212B
            *regWOBJLOG = byte;
            LogPpu("WOBJLOG=%02X. NYI", byte);
            return true;
        case eRegTM: // 0x212C
            *regTM = byte;
            LogPpu("Main Layers=%d,%d,%d,%d,%d", Bytes::GetBit<0>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<3>(byte), Bytes::GetBit<4>(byte));
            return true;
        case eRegTS: // 0x212D
            *regTS = byte;
            LogPpu("Subscreen Layers=%d,%d,%d,%d,%d", Bytes::GetBit<0>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<3>(byte), Bytes::GetBit<4>(byte));
            return true;
        case eRegTMW: // 0x212E
            *regTMW = byte;
            LogPpu("Main Window Layers=%d,%d,%d,%d,%d", Bytes::GetBit<0>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<3>(byte), Bytes::GetBit<4>(byte));
            return true;
        case eRegTSW: // 0x212F
            *regTSW = byte;
            LogPpu("Subscreen Window Layers=%d,%d,%d,%d,%d", Bytes::GetBit<0>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<3>(byte), Bytes::GetBit<4>(byte));
            return true;
        case eRegCGWSEL: // 0x2130
            *regCGWSEL = byte;
            LogPpu("ForceBlack=%d ColorMath=%d SubscreenBG=%d DirectColor=%d", (byte >> 6), ((byte >> 4) & 0x03), ((byte >> 1) & 0x01), (byte & 0x01));
            return true;
        case eRegCGADSUB: // 0x2131
            *regCGADSUB = byte;
            LogPpu("CGMath=%02X", byte);
            return true;
        case eRegCOLDATA: // 0x2132
            *regCOLDATA = byte;
            LogPpu("COLDATA=%02X. NYI", byte);
            return true;
        case eRegSETINI: // 0x2133
            *regSETINI = byte;
            LogPpu("ExtSync=%d ExtBg=%d HiRes=%d Overscan=%d, ObjInterlace=%d ScreenInterlace=%d", Bytes::GetBit<7>(byte), Bytes::GetBit<6>(byte), Bytes::GetBit<3>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<0>(byte));
            return true;
        case eRegMPYL:
            *regMPYL = byte;
            return false;
        case eRegMPYM:
            *regMPYM = byte;
            return false;
        case eRegMPYH:
            *regMPYH = byte;
            return false;
        case eRegSLHV:
            *regSLHV = byte;
            return false;
        case eRegRDOAM:
            *regRDOAM = byte;
            return false;
        case eRegRDVRAML:
            *regRDVRAML = byte;
            return false;
        case eRegRDVRAMH:
            *regRDVRAMH = byte;
            return false;
        case eRegRDCGRAM:
            *regRDCGRAM = byte;
            return false;
        case eRegOPHCT:
            *regOPHCT = byte;
            return false;
        case eRegOPVCT:
            *regOPVCT = byte;
            return false;
        case eRegSTAT77:
            *regSTAT77 = byte;
            return false;
        case eRegSTAT78:
            *regSTAT78 = byte;
            return false;
        default:
            return false;
    }
    return false;
}


void Ppu::UpdateTimer(uint32_t value)
{
    clockCounter += value;

    if (clockCounter > 4 && clockCounter < 1096)
    {
        isHBlank = false;
        // Clear HBlank flag.
        *memory->GetBytePtr(eRegHVBJOY) &= 0xBF;
    }
    else if (clockCounter >= 1096 && clockCounter < 1364)
    {
        isHBlank = true;
        // Set HBlank flag.
        *memory->GetBytePtr(eRegHVBJOY) |= 0x40;
    }
    else if (clockCounter >= 1364)
    {
        clockCounter -= 1364;

        // TODO: Check for number of scanlines per screen in regSETINI.

        if (scanline < 224)
            DrawScanline(scanline);

        scanline++;
        if (scanline == 225)
        {
            isVBlank = true;
            // Set VBlank flags.
            *memory->GetBytePtr(eRegRDNMI) |= 0x80;
            *memory->GetBytePtr(eRegHVBJOY) |= 0x80;

            // If joypad auto read is enabled, toggle the busy flag.
            // TODO: This doesn't belong in the PPU, move it somewhere better.
            if (Bytes::GetBit<0>(memory->ReadRaw8Bit(eRegNMITIMEN)))
                *memory->GetBytePtr(eRegHVBJOY) |= 0x01;

            // Reset the Oam address value on VBlank, but not when in forced VBlank.
            if (!isForcedBlank)
                oamRwAddr = Bytes::Make16Bit((*regOAMADDH) & 0x01, *regOAMADDL) << 1; // Word address.

            DrawScreen();
        }
        else if (scanline == 228)
        {
            // If joypad auto read is enabled, toggle the busy flag.
            // TODO: This doesn't belong in the PPU, move it somewhere better.
            if (Bytes::GetBit<0>(memory->ReadRaw8Bit(eRegNMITIMEN)))
                *memory->GetBytePtr(eRegHVBJOY) &= ~0x01;
        }
        else if (scanline == 262)
        {
            scanline = 0;
            isVBlank = false;
            // Clear VBlank flags.
            *memory->GetBytePtr(eRegRDNMI) &= 0x7F;
            *memory->GetBytePtr(eRegHVBJOY) &= 0x7F;
        }
    }
}


uint32_t Ppu::ConvertBGR555toARGB888(uint16_t bgrColor)
{
    uint8_t r = bgrColor & 0x1F;
    r = (r << 3) | ((r >> 2) & 0x07);

    bgrColor >>= 5;
    uint8_t g = bgrColor & 0x1F;
    g = (g << 3) | ((g >> 2) & 0x07);

    bgrColor >>= 5;
    uint8_t b = bgrColor & 0x1F;
    b = (b << 3) | ((b >> 2) & 0x07);

    return 0xFF000000 | Bytes::Make24Bit(r, g, b);
}


void Ppu::DrawScanline(uint8_t scanline)
{
    DrawBackgroundScanline(scanline);
}


void Ppu::DrawBackgroundScanline(uint8_t scanline)
{
    // Draw only Mode0 BG1 for now.
    // TODO: Check tile size in BGMODE
    // TODO: Check tile size in BGnSC

    // tilesetData is the actual pixel/palette-index data for a tile.
    const uint16_t tilesetDataOffset = (*regBG12NBA & 0x0F) << 13;
    const uint8_t *tilesetData = &vram[tilesetDataOffset];

    // tilemap is an index into tileData, as well as priority and flip data.
    //const uint8_t hTilemapCount = Bytes::GetBit<0>(*regBG1SC);
    //const uint8_t vTilemapCount = Bytes::GetBit<1>(*regBG1SC);
    const uint16_t tilemapOffset = (*regBG1SC & 0xFC) << 11;
    const uint16_t *tilemap = reinterpret_cast<const uint16_t*>(&vram[tilemapOffset]);

    const uint32_t TILE_SIZE = 8; // TODO: check actual tile size.
    const uint32_t TILEMAP_WIDTH = 32;
    uint32_t tileY = scanline / TILE_SIZE;
    for (int i = 0; i < SCREEN_X; i++)
    {
        uint32_t x = i / 2; // in 256 resolution mode.
        uint32_t tileX = x / TILE_SIZE;

        uint16_t tilemapEntry = tilemap[tileX + (tileY * TILEMAP_WIDTH)];
        uint32_t tileId = (tilemapEntry & 0x3FF) << 1; // Word address
        // TODO: Handle priority, palette, and H/V flip.
        uint32_t tileDataOffset = (tileId * TILE_SIZE) + ((scanline & 0x07) * 2);
        const uint8_t *tileData = &tilesetData[tileDataOffset];

        uint8_t lowBit = ((tileData[0] >> (7 - (x & 7))) & 0x01);
        uint8_t highBit = ((tileData[1] >> (7 - (x & 7))) & 0x01);
        uint8_t pixelVal = lowBit | (highBit << 1);
        uint32_t color = palette[pixelVal];

        uint32_t pixelOffset = ((scanline * 2) * SCREEN_X) + i;
        frameBuffer[pixelOffset] = color;
        frameBuffer[pixelOffset + (SCREEN_X)] = color;
    }
}


void Ppu::DrawScreen()
{
    displayInterface->FrameReady(frameBuffer);
}


void Ppu::DrawFullScreen()
{
    for (int i = 0; i < 224; i++)
    {
        DrawScanline(i);
    }
    DrawScreen();
}