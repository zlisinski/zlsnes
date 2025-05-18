#include "Bgr555.h"
#include "IoRegisters.h"
#include "DebuggerInterface.h"
#include "Memory.h"
#include "Ppu.h"
#include "PpuConstants.h"
#include "Timer.h"


uint16_t Ppu::PixelInfo::color0 = 0;


Ppu::Ppu(Memory *memory, Timer *timer, DisplayInterface *displayInterface, DebuggerInterface *debuggerInterface) :
    memory(memory),
    timer(timer),
    debuggerInterface(debuggerInterface),
    displayInterface(displayInterface),
    regINIDISP(memory->RequestOwnership(eRegINIDISP, this)),
    regOBJSEL(memory->RequestOwnership(eRegOBJSEL, this)),
    regOAMADDL(memory->RequestOwnership(eRegOAMADDL, this)),
    regOAMADDH(memory->RequestOwnership(eRegOAMADDH, this)),
    regOAMDATA(memory->RequestOwnership(eRegOAMDATA, this)),
    regBGMODE(memory->RequestOwnership(eRegBGMODE, this)),
    regMOSAIC(memory->RequestOwnership(eRegMOSAIC, this)),
    regBG1SC(memory->RequestOwnership(eRegBG1SC, this)),
    regBG2SC(memory->RequestOwnership(eRegBG2SC, this)),
    regBG3SC(memory->RequestOwnership(eRegBG3SC, this)),
    regBG4SC(memory->RequestOwnership(eRegBG4SC, this)),
    regBG12NBA(memory->RequestOwnership(eRegBG12NBA, this)),
    regBG34NBA(memory->RequestOwnership(eRegBG34NBA, this)),
    regBG1HOFS(memory->RequestOwnership(eRegBG1HOFS, this)),
    regBG1VOFS(memory->RequestOwnership(eRegBG1VOFS, this)),
    regBG2HOFS(memory->RequestOwnership(eRegBG2HOFS, this)),
    regBG2VOFS(memory->RequestOwnership(eRegBG2VOFS, this)),
    regBG3HOFS(memory->RequestOwnership(eRegBG3HOFS, this)),
    regBG3VOFS(memory->RequestOwnership(eRegBG3VOFS, this)),
    regBG4HOFS(memory->RequestOwnership(eRegBG4HOFS, this)),
    regBG4VOFS(memory->RequestOwnership(eRegBG4VOFS, this)),
    regVMAIN(memory->RequestOwnership(eRegVMAIN, this)),
    regVMADDL(memory->RequestOwnership(eRegVMADDL, this)),
    regVMADDH(memory->RequestOwnership(eRegVMADDH, this)),
    regVMDATAL(memory->RequestOwnership(eRegVMDATAL, this)),
    regVMDATAH(memory->RequestOwnership(eRegVMDATAH, this)),
    regM7SEL(memory->RequestOwnership(eRegM7SEL, this)),
    regM7A(memory->RequestOwnership(eRegM7A, this)),
    regM7B(memory->RequestOwnership(eRegM7B, this)),
    regM7C(memory->RequestOwnership(eRegM7C, this)),
    regM7D(memory->RequestOwnership(eRegM7D, this)),
    regM7X(memory->RequestOwnership(eRegM7X, this)),
    regM7Y(memory->RequestOwnership(eRegM7Y, this)),
    regCGADD(memory->RequestOwnership(eRegCGADD, this)),
    regCGDATA(memory->RequestOwnership(eRegCGDATA, this)),
    regW12SEL(memory->RequestOwnership(eRegW12SEL, this)),
    regW34SEL(memory->RequestOwnership(eRegW34SEL, this)),
    regWOBJSEL(memory->RequestOwnership(eRegWOBJSEL, this)),
    regWH0(memory->RequestOwnership(eRegWH0, this)),
    regWH1(memory->RequestOwnership(eRegWH1, this)),
    regWH2(memory->RequestOwnership(eRegWH2, this)),
    regWH3(memory->RequestOwnership(eRegWH3, this)),
    regWBGLOG(memory->RequestOwnership(eRegWBGLOG, this)),
    regWOBJLOG(memory->RequestOwnership(eRegWOBJLOG, this)),
    regTM(memory->RequestOwnership(eRegTM, this)),
    regTS(memory->RequestOwnership(eRegTS, this)),
    regTMW(memory->RequestOwnership(eRegTMW, this)),
    regTSW(memory->RequestOwnership(eRegTSW, this)),
    regCGWSEL(memory->RequestOwnership(eRegCGWSEL, this)),
    regCGADSUB(memory->RequestOwnership(eRegCGADSUB, this)),
    regCOLDATA(memory->RequestOwnership(eRegCOLDATA, this)),
    regSETINI(memory->RequestOwnership(eRegSETINI, this)),
    regMPYL(memory->RequestOwnership(eRegMPYL, this)),
    regMPYM(memory->RequestOwnership(eRegMPYM, this)),
    regMPYH(memory->RequestOwnership(eRegMPYH, this)),
    regSLHV(memory->RequestOwnership(eRegSLHV, this)),
    regRDOAM(memory->RequestOwnership(eRegRDOAM, this)),
    regRDVRAML(memory->RequestOwnership(eRegRDVRAML, this)),
    regRDVRAMH(memory->RequestOwnership(eRegRDVRAMH, this)),
    regRDCGRAM(memory->RequestOwnership(eRegRDCGRAM, this)),
    regOPHCT(memory->RequestOwnership(eRegOPHCT, this)),
    regOPVCT(memory->RequestOwnership(eRegOPVCT, this)),
    regSTAT77(memory->RequestOwnership(eRegSTAT77, this)),
    regSTAT78(memory->RequestOwnership(eRegSTAT78, this))
{
    timer->AttachHBlankObserver(this);
    timer->AttachVBlankObserver(this);
}


void Ppu::LatchCounters(bool force)
{
    // Bit 7 of regWRIO must be set when latching via read from regSLHV.
    // Bit 7 going from 1 to 0 also triggers latching. In that case force is set to true.
    if (force || Bytes::TestBit<7>(memory->ReadRaw8Bit(eRegWRIO)))
    {
        hCount = timer->GetHCount();
        vCount = timer->GetVCount();
        Bytes::SetBit<6>(regSTAT78);
    }
}


void Ppu::ToggleLayer(int layer, bool enabled)
{
    if (layer >= eBG1 && layer <= eOBJ)
        enableLayer[layer] = enabled;
}


uint8_t Ppu::ReadRegister(EIORegisters ioReg)
{
    LogPpu("Ppu::ReadRegister %04X", ioReg);

    switch (ioReg)
    {
        case eRegOAMDATA: // 0x2104
        case eRegBGMODE:  // 0x2105
        case eRegMOSAIC:  // 0x2106
        case eRegBG2SC:   // 0x2108
        case eRegBG3SC:   // 0x2109
        case eRegBG4SC:   // 0x210A
        case eRegBG4VOFS: // 0x2114
        case eRegVMAIN:   // 0x2115
        case eRegVMADDL:  // 0x2116
        case eRegVMDATAL: // 0x2118
        case eRegVMDATAH: // 0x2119
        case eRegM7SEL:   // 0x211A
        case eRegW34SEL:  // 0x2124
        case eRegWOBJSEL: // 0x2125
        case eRegWH0:     // 0x2126
        case eRegWH2:     // 0x2128
        case eRegWH3:     // 0x2129
        case eRegWBGLOG:  // 0x212A
            LogPpu("Read from open bus register %04X", ioReg);
            return ppu1OpenBus;

        case eRegINIDISP: // 0x2100
        case eRegOBJSEL:  // 0x2101
        case eRegOAMADDL: // 0x2102
        case eRegOAMADDH: // 0x2103
        case eRegBG1SC:   // 0x2107
        case eRegBG12NBA: // 0x210B
        case eRegBG34NBA: // 0x210C
        case eRegBG1HOFS: // 0x210D
        case eRegBG1VOFS: // 0x210E
        case eRegBG2HOFS: // 0x210F
        case eRegBG2VOFS: // 0x2110
        case eRegBG3HOFS: // 0x2111
        case eRegBG3VOFS: // 0x2112
        case eRegBG4HOFS: // 0x2113
        case eRegVMADDH:  // 0x2117
        case eRegM7A:     // 0x211B
        case eRegM7B:     // 0x211C
        case eRegM7C:     // 0x211D
        case eRegM7D:     // 0x211E
        case eRegM7X:     // 0x211F
        case eRegM7Y:     // 0x2120
        case eRegCGADD:   // 0x2121
        case eRegCGDATA:  // 0x2122
        case eRegW12SEL:  // 0x2123
        case eRegWH1:     // 0x2127
        case eRegWOBJLOG: // 0x212B
        case eRegTM:      // 0x212C
        case eRegTS:      // 0x212D
        case eRegTMW:     // 0x212E
        case eRegTSW:     // 0x212F
        case eRegCGWSEL:  // 0x2130
        case eRegCGADSUB: // 0x2131
        case eRegCOLDATA: // 0x2132
        case eRegSETINI:  // 0x2133
            LogPpu("Read from open bus register %04X", ioReg);
            return memory->GetOpenBusValue();

        case eRegMPYL: // 0x2134
            return (ppu1OpenBus = regMPYL);

        case eRegMPYM: // 0x2135
            return (ppu1OpenBus = regMPYM);

        case eRegMPYH: // 0x2136
            return (ppu1OpenBus = regMPYH);

        case eRegSLHV: // 0x2137
            LatchCounters();
            return memory->GetOpenBusValue();

        case eRegRDOAM: // 0x2138
        {
            uint8_t byte = oam[oamRwAddr];
            LogPpu("Read from oam %04X=%02X", oamRwAddr, byte);

            oamRwAddr = (oamRwAddr + 1) % oam.size();

            return (ppu1OpenBus = byte);
        }

        case eRegRDVRAML: // 0x2139
        {
            uint8_t byte = vramPrefetch[0];
            LogPpu("Read from vram %04X=%02X", vramRwAddr, byte);

            if (!isVramIncrementOnHigh)
            {
                // This happens before the address increment.
                vramPrefetch[0] = vram[vramRwAddr];
                vramPrefetch[1] = vram[(vramRwAddr + 1) % vram.size()];

                // This is a word address, so left shift 1 to get the byte address.
                vramRwAddr += vramIncrement << 1;
            }

            return (ppu1OpenBus = byte);
        }

        case eRegRDVRAMH: // 0x213A
        {
            uint8_t byte = vramPrefetch[1];
            LogPpu("Read from vram %04X=%02X", vramRwAddr + 1, byte);

            if (isVramIncrementOnHigh)
            {
                // This happens before the address increment.
                vramPrefetch[0] = vram[vramRwAddr];
                vramPrefetch[1] = vram[(vramRwAddr + 1) % vram.size()];

                // This is a word address, so left shift 1 to get the byte address.
                vramRwAddr += vramIncrement << 1;
            }

            return (ppu1OpenBus = byte);
        }

        case eRegRDCGRAM: // 0x213B
            LogPpu("Read from RDCGRAM NYI");
            return (ppu2OpenBus = regRDCGRAM);

        case eRegOPHCT: // 0x213C
            if (hCountFlipflop)
                regOPHCT = Bytes::GetBit<8>(hCount) | (ppu2OpenBus & 0xFE);
            else
                regOPHCT = Bytes::GetByte<0>(hCount);
            hCountFlipflop = !hCountFlipflop;
            return (ppu2OpenBus = regOPHCT);

        case eRegOPVCT: // 0x213D
            if (vCountFlipflop)
                regOPVCT = Bytes::GetBit<8>(vCount) | (ppu2OpenBus & 0xFE);
            else
                regOPVCT = Bytes::GetByte<0>(vCount);
            vCountFlipflop = !vCountFlipflop;
            return (ppu2OpenBus = regOPVCT);

        case eRegSTAT77: // 0x213E
            return (ppu1OpenBus = ((regSTAT77 & 0xE0) | (ppu1OpenBus & 0x10) | 0x01));

        case eRegSTAT78: // 0x213F
        {
            uint8_t value = (regSTAT78 & 0xD0) | (ppu2OpenBus & 0x20) | 0x03;
            Bytes::ClearBit<6>(regSTAT78);
            hCountFlipflop = false;
            vCountFlipflop = false;
            return (ppu2OpenBus = value);
        }

        default:
            throw std::range_error(fmt("Ppu doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Ppu::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogPpu("Ppu::WriteRegister %04X, %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegINIDISP: // 0x2100
            regINIDISP = byte;
            isForcedBlank = byte & 0x80;
            brightness = byte & 0x0F;
            // TODO: reset oamRwAddr if this is written on the first scanline of vblank (225/240).
            LogPpu("ForcedBlank=%d Brightness=%d", isForcedBlank, brightness);
            return true;

        case eRegOBJSEL: // 0x2101
            regOBJSEL = byte;
            objSize = byte >> 5;
            //objGap = (byte >> 3) & 0x03;
            objBaseAddr[0] = (byte & 0x03) << 14;
            objBaseAddr[1] = objBaseAddr[0] + ((((byte >> 3) & 0x03) + 1) << 13);
            LogPpu("OBJSEL=%02X. objSize=%dx%d/%dx%d objBaseAddr[0]=%04X objBaseAddr[1]=%04X", byte, OBJ_H_SIZE_LOOKUP[objSize][0], OBJ_V_SIZE_LOOKUP[objSize][0], OBJ_H_SIZE_LOOKUP[objSize][1], OBJ_H_SIZE_LOOKUP[objSize][1], objBaseAddr[0], objBaseAddr[1]);
            return true;

        case eRegOAMADDL: // 0x2102
            regOAMADDL = byte;
            // This is a word address, so left shift 1 to get the byte address.
            oamRwAddr = Bytes::Make16Bit(regOAMADDH & 0x01, byte) << 1;
            LogPpu("oamRwAddr=%04X", oamRwAddr);
            return true;

        case eRegOAMADDH: // 0x2103
            regOAMADDH = byte;
            // This is a word address, so left shift 1 to get the byte address.
            oamRwAddr = Bytes::Make16Bit(byte & 0x01, regOAMADDL) << 1;
            objPriorityRotation = Bytes::GetBit<7>(byte);
            LogPpu("OAMADDH=%02X oamRwAddr=%04X objPriorityRotation=%d", byte, oamRwAddr, objPriorityRotation);
            return true;

        case eRegOAMDATA: // 0x2104
            regOAMDATA = byte;

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
            regBGMODE = byte;
            bgMode = byte & 0x07;
            bgMode1Bg3Priority = Bytes::GetBit<3>(byte);
            bgChrSize[0] = 8 << Bytes::GetBit<4>(byte);
            bgChrSize[1] = 8 << Bytes::GetBit<5>(byte);
            bgChrSize[2] = 8 << Bytes::GetBit<6>(byte);
            bgChrSize[3] = 8 << Bytes::GetBit<7>(byte);
            LogPpu("bgMode=%d bg3Prio=%d bgChrSize=%d,%d,%d,%d", bgMode, bgMode1Bg3Priority, bgChrSize[0], bgChrSize[1], bgChrSize[2], bgChrSize[3]);
            if (bgMode == 2 || bgMode == 4 || bgMode == 6)
                LogWarning("Mode %d OPT NYI", bgMode);
            return true;

        case eRegMOSAIC: // 0x2106
            regMOSAIC = byte;
            bgEnableMosaic[eBG1] = Bytes::TestBit<0>(byte);
            bgEnableMosaic[eBG2] = Bytes::TestBit<1>(byte);
            bgEnableMosaic[eBG3] = Bytes::TestBit<2>(byte);
            bgEnableMosaic[eBG4] = Bytes::TestBit<3>(byte);
            bgMosaicSize = (byte >> 4) + 1;

            // When set mid frame, the top of the mosaic block is the current scanline.
            bgMosaicStartScanline = scanline;

            LogPpu("MosaicSize=%d MosaicLayers=%d,%d,%d,%d", bgMosaicSize, bgEnableMosaic[eBG1], bgEnableMosaic[eBG2],
                   bgEnableMosaic[eBG3], bgEnableMosaic[eBG4]);
            return true;

        case eRegBG1SC: // 0x2107
            regBG1SC = byte;
            bgTilemapAddr[eBG1] = (byte & 0xFC) << 9;
            bgTilemapWidth[eBG1] = 32 << Bytes::GetBit<0>(byte);
            bgTilemapHeight[eBG1] = 32 << Bytes::GetBit<1>(byte);
            LogPpu("BG1 Address=%04X Size=%dx%d", bgTilemapAddr[0], bgTilemapWidth[eBG1], bgTilemapHeight[eBG1]);
            return true;

        case eRegBG2SC: // 0x2108
            regBG2SC = byte;
            bgTilemapAddr[eBG2] = (byte & 0xFC) << 9;
            bgTilemapWidth[eBG2] = 32 << Bytes::GetBit<0>(byte);
            bgTilemapHeight[eBG2] = 32 << Bytes::GetBit<1>(byte);
            LogPpu("BG2 Address=%04X Size=%dx%d", bgTilemapAddr[0], bgTilemapWidth[eBG2], bgTilemapHeight[eBG2]);
            return true;

        case eRegBG3SC: // 0x2109
            regBG3SC = byte;
            bgTilemapAddr[eBG3] = (byte & 0xFC) << 9;
            bgTilemapWidth[eBG3] = 32 << Bytes::GetBit<0>(byte);
            bgTilemapHeight[eBG3] = 32 << Bytes::GetBit<1>(byte);
            LogPpu("BG3 Address=%04X Size=%dx%d", bgTilemapAddr[0], bgTilemapWidth[eBG3], bgTilemapHeight[eBG3]);
            return true;

        case eRegBG4SC: // 0x210A
            regBG4SC = byte;
            bgTilemapAddr[eBG4] = (byte & 0xFC) << 9;
            bgTilemapWidth[eBG4] = 32 << Bytes::GetBit<0>(byte);
            bgTilemapHeight[eBG4] = 32 << Bytes::GetBit<1>(byte);
            LogPpu("BG4 Address=%04X Size=%dx%d", bgTilemapAddr[0], bgTilemapWidth[eBG4], bgTilemapHeight[eBG4]);
            return true;

        case eRegBG12NBA: // 0x210B
            regBG12NBA = byte;
            bgChrAddr[0] = (byte & 0x0F) << 13;
            bgChrAddr[1] = (byte & 0xF0) << 9;
            LogPpu("Tile Address BG1=%04x BG2=%04X", bgChrAddr[0], bgChrAddr[1]);
            return true;

        case eRegBG34NBA: // 0x210C
            regBG34NBA = byte;
            bgChrAddr[2] = (byte & 0x0F) << 13;
            bgChrAddr[3] = (byte & 0xF0) << 9;
            LogPpu("Tile Address BG3=%04x BG4=%04X", bgChrAddr[2], bgChrAddr[3]);
            return true;

        case eRegBG1HOFS: // 0x210D
            regBG1HOFS = byte;
            SetBgHOffsetWriteTwice(eBG1, byte);

            // M7HOFS and BG1HOFS share the same address.
            m7HOffset = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch) << 3) >> 3; // Sign extend 13-bit to 16-bit.
            m7Latch = byte;
            return true;

        case eRegBG1VOFS: // 0x210E
            regBG1VOFS = byte;
            SetBgVOffsetWriteTwice(eBG1, byte);

            // M7VOFS and BG1VOFS share the same address.
            m7VOffset = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch) << 3) >> 3; // Sign extend 13-bit to 16-bit.
            m7Latch = byte;
            return true;

        case eRegBG2HOFS: // 0x210F
            regBG2HOFS = byte;
            SetBgHOffsetWriteTwice(eBG2, byte);
            return true;

        case eRegBG2VOFS: // 0x2110
            regBG2VOFS = byte;
            SetBgVOffsetWriteTwice(eBG2, byte);
            return true;

        case eRegBG3HOFS: // 0x2111
            regBG3HOFS = byte;
            SetBgHOffsetWriteTwice(eBG3, byte);
            return true;

        case eRegBG3VOFS: // 0x2112
            regBG3VOFS = byte;
            SetBgVOffsetWriteTwice(eBG3, byte);
            return true;

        case eRegBG4HOFS: // 0x2113
            regBG4HOFS = byte;
            SetBgHOffsetWriteTwice(eBG4, byte);
            return true;

        case eRegBG4VOFS: // 0x2114
            regBG4VOFS = byte;
            SetBgVOffsetWriteTwice(eBG4, byte);
            return true;

        case eRegVMAIN: // 0x2115
            regVMAIN = byte;
            isVramIncrementOnHigh = byte & 0x80;
            switch (byte & 0x03)
            {
                case 0: vramIncrement = 1; break;
                case 1: vramIncrement = 32; break;
                case 2: case 3: vramIncrement = 128; break;
            }
            vramAddrTranslation = (byte >> 2) & 0x03;
            LogPpu("Increment VRAM by %d after reading %s byte, translate %d bits", vramIncrement, isVramIncrementOnHigh ? "High" : "Low", vramAddrTranslation);
            return true;

        case eRegVMADDL: // 0x2116
            regVMADDL = byte;

            // This is a word address, so left shift 1 to get the byte address.
            vramRwAddr = Bytes::Make16Bit(regVMADDH, byte) << 1;
            LogPpu("vramRwAddr=%04X", vramRwAddr);

            // Prefetch the bytes when the address changes.
            vramPrefetch[0] = vram[vramRwAddr];
            vramPrefetch[1] = vram[(vramRwAddr + 1) % vram.size()];

            return true;

        case eRegVMADDH: // 0x2117
            regVMADDH = byte;

            // This is a word address, so left shift 1 to get the byte address.
            vramRwAddr = Bytes::Make16Bit(byte, regVMADDL) << 1;
            LogPpu("vramRwAddr=%04X", vramRwAddr);

            // Prefetch the bytes when the address changes.
            vramPrefetch[0] = vram[vramRwAddr];
            vramPrefetch[1] = vram[(vramRwAddr + 1) % vram.size()];

            return true;

        case eRegVMDATAL: // 0x2118
        {
            regVMDATAL = byte;
            uint16_t addr = TranslateVramAddress(vramRwAddr, vramAddrTranslation);
            vram[addr] = byte;
            LogPpu("Write to vram %04X=%02X", addr, byte);
            if (!isVramIncrementOnHigh)
            {
                // This is a word address, so left shift 1 to get the byte address.
                vramRwAddr += vramIncrement << 1;
            }
            return true;
        }

        case eRegVMDATAH: // 0x2119
        {
            regVMDATAH = byte;
            uint16_t addr = TranslateVramAddress(vramRwAddr, vramAddrTranslation) + 1;
            vram[addr] = byte;
            LogPpu("Write to vram %04X=%02X", addr, byte);
            if (isVramIncrementOnHigh)
            {
                // This is a word address, so left shift 1 to get the byte address.
                vramRwAddr += vramIncrement << 1;
            }
            return true;
        }

        case eRegM7SEL:
            regM7SEL = byte;
            m7ExtendedFill = Bytes::TestBit<7>(byte);
            m7FillColorTile0 = Bytes::TestBit<6>(byte);
            m7FlipY = Bytes::TestBit<1>(byte);
            m7FlipX = Bytes::TestBit<0>(byte);
            LogPpu("M7SEL=%02X", byte);
            return true;

        case eRegM7A: // 0x211B
            regM7A = byte;
            m7a = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch));
            m7Latch = byte;
            // Writes to this register also performs multiplication.
            M7Multiply();
            LogPpu("M7A=%04X", m7a);
            return true;

        case eRegM7B: // 0x211C
            regM7B = byte;
            m7b = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch));
            m7Latch = byte;
            // Writes to this register also performs multiplication.
            M7Multiply();
            LogPpu("M7B=%04X", m7b);
            return true;

        case eRegM7C: // 0x211D
            regM7C = byte;
            m7c = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch));
            m7Latch = byte;
            LogPpu("M7C=%04X", m7c);
            return true;

        case eRegM7D: // 0x211E
            regM7D = byte;
            m7d = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch));
            m7Latch = byte;
            LogPpu("M7D=%04X", m7d);
            return true;

        case eRegM7X: // 0x211F
            regM7X = byte;
            m7x = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch) << 3) >> 3; // Sign extend 13-bit to 16-bit.
            m7Latch = byte;
            LogPpu("M7X=%04X", m7x);
            return true;

        case eRegM7Y: // 0x2120
            regM7Y = byte;
            m7y = static_cast<int16_t>(Bytes::Make16Bit(byte, m7Latch) << 3) >> 3; // Sign extend 13-bit to 16-bit.
            m7Latch = byte;
            LogPpu("M7Y=%04X", m7y);
            return true;

        case eRegCGADD: // 0x2121
            regCGADD = byte;
            // This is a word address, so left shift 1 to get the byte address.
            // Since the low bit is always 0, this also resets the read/write twice sequence on CGDATA/RDCGRAM.
            cgramRwAddr = byte << 1;
            LogPpu("Palette color index=%04X", cgramRwAddr);
            return true;

        case eRegCGDATA: // 0x2122
            regCGDATA = byte;
            // The first write gets saved, and both bytes get written to cgram on the second write to the register.
            if ((cgramRwAddr & 0x01) == 0)
            {
                cgramLatch = byte;
                LogPpu("Saving cgram byte %02X", byte);
            }
            else
            {
                byte &= 0x7F;
                cgram[cgramRwAddr - 1] = cgramLatch;
                cgram[cgramRwAddr] = byte;
                LogPpu("Writing word to cgram %04X %02X%02X", cgramRwAddr - 1, byte, cgramLatch);

                uint16_t color = Bytes::Make16Bit(byte, cgramLatch);

                if ((cgramRwAddr >> 1) == 0)
                    PixelInfo::color0 = color;
            }
            cgramRwAddr = (cgramRwAddr + 1) & 0x1FF;
            return true;

        case eRegW12SEL: // 0x2123
            if (byte != regW12SEL)
                windowChanged = true;
            regW12SEL = byte;
            bgInvertWindow[eBG1][0] = Bytes::TestBit<0>(byte);
            bgEnableWindow[eBG1][0] = Bytes::TestBit<1>(byte);
            bgInvertWindow[eBG1][1] = Bytes::TestBit<2>(byte);
            bgEnableWindow[eBG1][1] = Bytes::TestBit<3>(byte);
            bgInvertWindow[eBG2][0] = Bytes::TestBit<4>(byte);
            bgEnableWindow[eBG2][0] = Bytes::TestBit<5>(byte);
            bgInvertWindow[eBG2][1] = Bytes::TestBit<6>(byte);
            bgEnableWindow[eBG2][1] = Bytes::TestBit<7>(byte);
            LogPpu("W12SEL=%02X Bg1En=%d,%d Bg1In=%d,%d Bg2En=%d,%d Bg2In=%d,%d", byte,
                   bgEnableWindow[eBG1][0], bgEnableWindow[eBG1][1], bgInvertWindow[eBG1][0], bgInvertWindow[eBG1][1],
                   bgEnableWindow[eBG2][0], bgEnableWindow[eBG2][1], bgInvertWindow[eBG2][0], bgInvertWindow[eBG2][1]);
            return true;

        case eRegW34SEL: // 0x2124
            if (byte != regW34SEL)
                windowChanged = true;
            regW34SEL = byte;
            bgInvertWindow[eBG3][0] = Bytes::TestBit<0>(byte);
            bgEnableWindow[eBG3][0] = Bytes::TestBit<1>(byte);
            bgInvertWindow[eBG3][1] = Bytes::TestBit<2>(byte);
            bgEnableWindow[eBG3][1] = Bytes::TestBit<3>(byte);
            bgInvertWindow[eBG4][0] = Bytes::TestBit<4>(byte);
            bgEnableWindow[eBG4][0] = Bytes::TestBit<5>(byte);
            bgInvertWindow[eBG4][1] = Bytes::TestBit<6>(byte);
            bgEnableWindow[eBG4][1] = Bytes::TestBit<7>(byte);
            LogPpu("W34SEL=%02X Bg3En=%d,%d Bg3In=%d,%d Bg4En=%d,%d Bg4In=%d,%d", byte,
                   bgEnableWindow[eBG3][0], bgEnableWindow[eBG3][1], bgInvertWindow[eBG3][0], bgInvertWindow[eBG3][1],
                   bgEnableWindow[eBG4][0], bgEnableWindow[eBG4][1], bgInvertWindow[eBG4][0], bgInvertWindow[eBG4][1]);
            return true;

        case eRegWOBJSEL: // 0x2125
            if (byte != regWOBJSEL)
                windowChanged = true;
            regWOBJSEL = byte;
            bgInvertWindow[eOBJ][0] = Bytes::TestBit<0>(byte);
            bgEnableWindow[eOBJ][0] = Bytes::TestBit<1>(byte);
            bgInvertWindow[eOBJ][1] = Bytes::TestBit<2>(byte);
            bgEnableWindow[eOBJ][1] = Bytes::TestBit<3>(byte);
            bgInvertWindow[eCOL][0] = Bytes::TestBit<4>(byte);
            bgEnableWindow[eCOL][0] = Bytes::TestBit<5>(byte);
            bgInvertWindow[eCOL][1] = Bytes::TestBit<6>(byte);
            bgEnableWindow[eCOL][1] = Bytes::TestBit<7>(byte);
            LogPpu("WOBJSEL=%02X ObjEn=%d,%d ObjIn=%d,%d ColEn=%d,%d ColIn=%d,%d", byte,
                   bgEnableWindow[eOBJ][0], bgEnableWindow[eOBJ][1], bgInvertWindow[eOBJ][0], bgInvertWindow[eOBJ][1],
                   bgEnableWindow[eCOL][0], bgEnableWindow[eCOL][1], bgInvertWindow[eCOL][0], bgInvertWindow[eCOL][1]);
            return true;

        case eRegWH0: // 0x2126
            if (byte != regWH0)
                windowChanged = true;
            regWH0 = byte;
            windowLeft[0] = byte;
            LogPpu("WH0 Window 1 Left=%02X", byte);
            return true;

        case eRegWH1: // 0x2127
            if (byte != regWH1)
                windowChanged = true;
            regWH1 = byte;
            windowRight[0] = byte;
            LogPpu("WH1 Window 1 Right=%02X", byte);
            return true;

        case eRegWH2: // 0x2128
            if (byte != regWH2)
                windowChanged = true;
            regWH2 = byte;
            windowLeft[1] = byte;
            LogPpu("WH2 Window 2 Left=%02X", byte);
            return true;

        case eRegWH3: // 0x2129
            if (byte != regWH3)
                windowChanged = true;
            regWH3 = byte;
            windowRight[1] = byte;
            LogPpu("WH3 Window 2 Right=%02X", byte);
            return true;

        case eRegWBGLOG: // 0x212A
            if (byte != regWBGLOG)
                windowChanged = true;
            regWBGLOG = byte;
            bgWindowMask[eBG1] = byte & 0x03;
            bgWindowMask[eBG2] = (byte >> 2) & 0x03;
            bgWindowMask[eBG3] = (byte >> 4) & 0x03;
            bgWindowMask[eBG4] = byte >> 6;
            LogPpu("WBGLOG=%02X bgMask=%d,%d,%d,%d", byte, bgWindowMask[eBG1], bgWindowMask[eBG2], bgWindowMask[eBG3], bgWindowMask[eBG4]);
            return true;

        case eRegWOBJLOG: // 0x212B
            if (byte != regWOBJLOG)
                windowChanged = true;
            regWOBJLOG = byte;
            bgWindowMask[eOBJ] = byte & 0x03;
            bgWindowMask[eCOL] = (byte >> 2) & 0x03;
            LogPpu("WOBJLOG=%02X objMask=%d colMask=%d", byte, bgWindowMask[eOBJ], bgWindowMask[eCOL]);
            return true;

        case eRegTM: // 0x212C
            regTM = byte;
            mainScreenLayerEnabled[eBG1] = Bytes::GetBit<0>(byte);
            mainScreenLayerEnabled[eBG2] = Bytes::GetBit<1>(byte);
            mainScreenLayerEnabled[eBG3] = Bytes::GetBit<2>(byte);
            mainScreenLayerEnabled[eBG4] = Bytes::GetBit<3>(byte);
            mainScreenLayerEnabled[eOBJ] = Bytes::GetBit<4>(byte);
            LogPpu("Main Layers=%d,%d,%d,%d,%d", mainScreenLayerEnabled[eBG1], mainScreenLayerEnabled[eBG2],
                   mainScreenLayerEnabled[eBG3], mainScreenLayerEnabled[eBG4], mainScreenLayerEnabled[eOBJ]);
            return true;

        case eRegTS: // 0x212D
            regTS = byte;
            subScreenLayerEnabled[eBG1] = Bytes::GetBit<0>(byte);
            subScreenLayerEnabled[eBG2] = Bytes::GetBit<1>(byte);
            subScreenLayerEnabled[eBG3] = Bytes::GetBit<2>(byte);
            subScreenLayerEnabled[eBG4] = Bytes::GetBit<3>(byte);
            subScreenLayerEnabled[eOBJ] = Bytes::GetBit<4>(byte);
            LogPpu("Subscreen Layers=%d,%d,%d,%d,%d", subScreenLayerEnabled[eBG1], subScreenLayerEnabled[eBG2],
                   subScreenLayerEnabled[eBG3], subScreenLayerEnabled[eBG4], subScreenLayerEnabled[eOBJ]);
            return true;

        case eRegTMW: // 0x212E
            regTMW = byte;
            mainScreenWindowEnabled[eBG1] = Bytes::GetBit<0>(byte);
            mainScreenWindowEnabled[eBG2] = Bytes::GetBit<1>(byte);
            mainScreenWindowEnabled[eBG3] = Bytes::GetBit<2>(byte);
            mainScreenWindowEnabled[eBG4] = Bytes::GetBit<3>(byte);
            mainScreenWindowEnabled[eOBJ] = Bytes::GetBit<4>(byte);
            LogPpu("Main Window Layers=%d,%d,%d,%d,%d", mainScreenWindowEnabled[eBG1], mainScreenWindowEnabled[eBG2],
                   mainScreenWindowEnabled[eBG3], mainScreenWindowEnabled[eBG4], mainScreenWindowEnabled[eOBJ]);
            return true;

        case eRegTSW: // 0x212F
            regTSW = byte;
            subScreenWindowEnabled[eBG1] = Bytes::GetBit<0>(byte);
            subScreenWindowEnabled[eBG2] = Bytes::GetBit<1>(byte);
            subScreenWindowEnabled[eBG3] = Bytes::GetBit<2>(byte);
            subScreenWindowEnabled[eBG4] = Bytes::GetBit<3>(byte);
            subScreenWindowEnabled[eOBJ] = Bytes::GetBit<4>(byte);
            LogPpu("Subscreen Window Layers=%d,%d,%d,%d,%d", subScreenWindowEnabled[eBG1], subScreenWindowEnabled[eBG2],
                   subScreenWindowEnabled[eBG3], subScreenWindowEnabled[eBG4], subScreenWindowEnabled[eOBJ]);
            return true;

        case eRegCGWSEL: // 0x2130
            regCGWSEL = byte;
            colDirectMode = Bytes::TestBit<0>(byte);
            colAddend = Bytes::TestBit<1>(byte);
            preventColorMath = static_cast<EColorRegion>((byte >> 4) & 0x03);
            clipToBlack = static_cast<EColorRegion>(byte >> 6);
            LogPpu("CGWSEL=%02X clipToBlack=%d preventColorMath=%d addend=%d directColor=%d", byte,
                   (int)clipToBlack, (int)preventColorMath, colAddend, colDirectMode);
            if (colDirectMode)
                LogWarning("Direct color mode NYI");
            return true;

        case eRegCGADSUB: // 0x2131
            regCGADSUB = byte;
            bgColorMathEnable[eBG1] = Bytes::GetBit<0>(byte);
            bgColorMathEnable[eBG2] = Bytes::GetBit<1>(byte);
            bgColorMathEnable[eBG3] = Bytes::GetBit<2>(byte);
            bgColorMathEnable[eBG4] = Bytes::GetBit<3>(byte);
            bgColorMathEnable[eOBJ] = Bytes::GetBit<4>(byte);
            bgColorMathEnable[eCOL] = Bytes::GetBit<5>(byte);
            halfColorMath = Bytes::GetBit<6>(byte);
            colorSubtract = Bytes::GetBit<7>(byte);
            LogPpu("CGADSUB=%02X colMath=%d,%d,%d,%d,%d,%d half=%d sub=%d", byte,
                    bgColorMathEnable[eBG1], bgColorMathEnable[eBG2], bgColorMathEnable[eBG3], bgColorMathEnable[eBG4],
                    bgColorMathEnable[eOBJ], bgColorMathEnable[eCOL], halfColorMath, colorSubtract);
            return true;

        case eRegCOLDATA: // 0x2132
            regCOLDATA = byte;
            if (Bytes::TestBit<5>(byte))
                redChannel = byte & 0x1F;
            if (Bytes::TestBit<6>(byte))
                greenChannel = byte & 0x1F;
            if (Bytes::TestBit<7>(byte))
                blueChannel = byte & 0x1F;
            fixedColor = (blueChannel << 10) | (greenChannel << 5) | redChannel;
            LogPpu("COLDATA=%02X r=%d b=%d g=%d fixed=%04X", byte, redChannel, blueChannel, greenChannel, fixedColor);
            return true;

        case eRegSETINI: // 0x2133
            regSETINI = byte;
            LogPpu("ExtSync=%d ExtBg=%d HiRes=%d Overscan=%d, ObjInterlace=%d ScreenInterlace=%d NYI", Bytes::GetBit<7>(byte), Bytes::GetBit<6>(byte), Bytes::GetBit<3>(byte), Bytes::GetBit<2>(byte), Bytes::GetBit<1>(byte), Bytes::GetBit<0>(byte));
            return true;

        default:
            return false;
    }
    return false;
}


 void Ppu::ProcessHBlankStart(uint32_t scanline)
 {
    // We reached the end of the scanline, so draw it.
    // TODO: Check for number of scanlines per screen in regSETINI.
    if (scanline < 224)
        DrawScanline(scanline);
 }


void Ppu::ProcessHBlankEnd(uint32_t scanline)
{
    // We started a new scanline.
    this->scanline = scanline;
}


void Ppu::ProcessVBlankStart()
{
    // Reset the Oam address value on VBlank, but not when in forced blank.
    if (!isForcedBlank)
        oamRwAddr = Bytes::Make16Bit(regOAMADDH & 0x01, regOAMADDL) << 1; // Word address.

    DrawScreen();
}


void Ppu::ProcessVBlankEnd()
{
    // Clear sprite overflow flags.
    if (!isForcedBlank)
    {
        Bytes::ClearBit<6>(regSTAT77);
        Bytes::ClearBit<7>(regSTAT77);
    }

    // Reset which scanline is the starting vertical block for mosaic.
    bgMosaicStartScanline = 1;
}


void Ppu::GenerateWindowBitmaps()
{
    memset(windowBitmap, 0, sizeof(windowBitmap));

    for (int bg = 0; bg < 6; bg++)
    {
        if (bgEnableWindow[bg][0] && !bgEnableWindow[bg][1])
        {
            // Only window 0.
            GenerateWindowLayerBitmap(static_cast<EBgLayer>(bg), 0, windowBitmap[bg]);
        }
        else if (!bgEnableWindow[bg][0] && bgEnableWindow[bg][1])
        {
            // Only window 1.
            GenerateWindowLayerBitmap(static_cast<EBgLayer>(bg), 1, windowBitmap[bg]);
        }
        else if (bgEnableWindow[bg][0] && bgEnableWindow[bg][1])
        {
            // Both windows. Use combination logic.
            uint64_t other[4] = {0};
            GenerateWindowLayerBitmap(static_cast<EBgLayer>(bg), 0, windowBitmap[bg]);
            GenerateWindowLayerBitmap(static_cast<EBgLayer>(bg), 1, other);

            switch (bgWindowMask[bg])
            {
                case 0:
                    windowBitmap[bg][0] |= other[0];
                    windowBitmap[bg][1] |= other[1];
                    windowBitmap[bg][2] |= other[2];
                    windowBitmap[bg][3] |= other[3];
                    break;
                case 1:
                    windowBitmap[bg][0] &= other[0];
                    windowBitmap[bg][1] &= other[1];
                    windowBitmap[bg][2] &= other[2];
                    windowBitmap[bg][3] &= other[3];
                    break;
                case 2:
                    windowBitmap[bg][0] ^= other[0];
                    windowBitmap[bg][1] ^= other[1];
                    windowBitmap[bg][2] ^= other[2];
                    windowBitmap[bg][3] ^= other[3];
                    break;
                case 3:
                    windowBitmap[bg][0] = !(windowBitmap[bg][0] ^ other[0]);
                    windowBitmap[bg][1] = !(windowBitmap[bg][1] ^ other[1]);
                    windowBitmap[bg][2] = !(windowBitmap[bg][2] ^ other[2]);
                    windowBitmap[bg][3] = !(windowBitmap[bg][3] ^ other[3]);
                    break;
            }
        }
    }

    windowChanged = false;
}


void Ppu::GenerateWindowLayerBitmap(EBgLayer bg, uint8_t window, uint64_t *bitmap)
{
    if (windowLeft[window] > windowRight[window])
    {
        if (bgInvertWindow[bg][window])
            memset(bitmap, 0xFF, 32);
        return;
    }

    int startWord = windowLeft[window] / 64;
    int startBit = windowLeft[window] % 64;
    int endWord = windowRight[window] / 64;
    int endBit = windowRight[window] % 64;

    // Set bits in the first word
    bitmap[startWord] = ~0UL << startBit;

    if (startWord == endWord)
    {
        // Start and end word are the same.
        bitmap[startWord] &= ~0UL >> (63 - endBit);
    }
    else
    {
        // Set partial bits in end word.
        bitmap[endWord] = ~0UL >> (63 - endBit);

        // Set all bits in full words between start and end
        for (int i = startWord + 1; i < endWord; i++)
            bitmap[i] = ~0UL;
    }

    if (bgInvertWindow[bg][window])
    {
        bitmap[0] ^= ~0UL;
        bitmap[1] ^= ~0UL;
        bitmap[2] ^= ~0UL;
        bitmap[3] ^= ~0UL;
    }
}


bool Ppu::IsPointInsideWindow(EBgLayer bg, uint16_t screenX) const
{
    int word = screenX / 64;
    int bit = screenX % 64;

    return windowBitmap[bg][word] & (1UL << bit);
}


Ppu::WindowInfo Ppu::GetBgWindowValue(EBgLayer bg, uint16_t screenX) const
{
    WindowInfo info;

    info.isOnMainScreen = mainScreenWindowEnabled[bg];
    info.isOnSubScreen = subScreenWindowEnabled[bg];

    if (info.isOnMainScreen || info.isOnSubScreen)
        info.isInside = IsPointInsideWindow(bg, screenX);

    return info;
}


uint8_t Ppu::GetTilePixelData(uint16_t addr, uint8_t xOff, uint8_t yOff, uint8_t bpp) const
{
    const uint8_t *tileData = &vram[addr];

    xOff = 7 - xOff;
    // Two bytes per pixel.
    yOff = yOff << 1;

    uint8_t lowBit = (tileData[yOff] >> xOff) & 0x01;
    uint8_t highBit = (tileData[yOff + 1] >> xOff) & 0x01;
    uint8_t pixelVal = (highBit << 1) | lowBit;
    if (bpp >= 4)
    {
        uint8_t lowBit2 = (tileData[yOff + 0x10] >> xOff) & 0x01;
        uint8_t highBit2 = (tileData[yOff + 0x11] >> xOff) & 0x01;
        pixelVal |= (highBit2 << 3) | (lowBit2 << 2);
    }
    if (bpp == 8)
    {
        uint8_t lowBit3 = (tileData[yOff + 0x20] >> xOff) & 0x01;
        uint8_t highBit3 = (tileData[yOff + 0x21] >> xOff) & 0x01;
        uint8_t lowBit4 = (tileData[yOff + 0x30] >> xOff) & 0x01;
        uint8_t highBit4 = (tileData[yOff + 0x31] >> xOff) & 0x01;
        pixelVal |= (highBit4 << 7) | (lowBit4 << 6) | (highBit3 << 5) | (lowBit3 << 4);
    }

    return pixelVal;
}


uint16_t Ppu::GetBgTilemapEntry(EBgLayer bg, uint16_t tileX, uint16_t tileY) const
{
    // Compute the offset for 32x32 tilemap.
    uint16_t offset = (tileX & 0x1F) + ((tileY & 0x1F) * 32);

    // Adjust if either dimension is extended to 64, and the tile is in the extened area.
    if (tileX >= 32)
    {
        // Add 1K to get to next tilemap.
        offset += 0x400;
    }
    if (tileY >= 32)
    {
        // Add either 1K or 2K depending on whether the X is extended too.
        if (bgTilemapWidth[bg] == 64)
            offset += 0x800;
        else
            offset += 0x400;
    }

    // Each tile is a word, so double the offset.
    offset = bgTilemapAddr[bg] + (offset << 1);

    uint16_t tileData = Bytes::Make16Bit(vram[offset + 1], vram[offset]);
    return tileData;
}


Ppu::PixelInfo Ppu::GetBgPixelInfo(EBgLayer bg, uint16_t screenX, uint16_t screenY)
{
    PixelInfo info;
    uint8_t bpp = BG_BPP_LOOKUP[bgMode][bg];

    // Check if layer is disabled in emulator GUI.
    if (!enableLayer[bg])
        return info;

    info.isOnMainScreen = mainScreenLayerEnabled[bg];
    info.isOnSubScreen = subScreenLayerEnabled[bg];

    // Check if the layer is enabled for at least one screen and not disabled by the bgmode.
    if ((!info.isOnMainScreen && !info.isOnSubScreen) || bpp == 0)
        return info;

    WindowInfo window = GetBgWindowValue(bg, screenX);

    // Disable the pixel for the screen if it's inside the window and windows are enabled for the bg.
    info.isOnMainScreen &= !(window.isInside && window.isOnMainScreen);
    info.isOnSubScreen &= !(window.isInside && window.isOnSubScreen);

    // If all screens are inside the window, the pixel is never drawn.
    if (!info.isOnMainScreen && !info.isOnSubScreen)
        return info;

    int mosaicXOff = 0;
    int mosaicYOff = 0;
    if (bgEnableMosaic[bg] && bgMosaicSize > 1)
    {
        mosaicXOff = screenX % bgMosaicSize;
        mosaicYOff = (screenY - bgMosaicStartScanline) % bgMosaicSize;
    }

    const int tileXSize = IsHiRes() ? 16 : bgChrSize[bg];
    const int tileYSize = bgChrSize[bg];
    const int hOffset = IsHiRes() ? bgHOffset[bg] * 2 : bgHOffset[bg];
    const int vOffset = bgVOffset[bg];
    const int tileX = ((screenX + hOffset - mosaicXOff) / tileXSize) & (bgTilemapWidth[bg] - 1);
    const int tileY = ((screenY + vOffset - mosaicYOff) / tileYSize) & (bgTilemapHeight[bg] - 1);
    int xOff = (screenX + hOffset - mosaicXOff) & (tileXSize - 1);
    int yOff = (screenY + vOffset - mosaicYOff) & (tileYSize - 1);

    // Cache the tile since the next 8 pixels will use the same tile.
    BgTilemapCache &tile = bgTilemapCache[bg];
    if (tile.tileX != tileX || tile.tileY != tileY)
        tile = BgTilemapCache(GetBgTilemapEntry(bg, tileX, tileY), tileX, tileY);

    info.paletteId = tile.data.paletteId;
    info.priority = tile.data.priority;

    if (tile.data.flipX)
        xOff = (tileXSize - 1) - xOff;
    if (tile.data.flipY)
        yOff = (tileYSize - 1) - yOff;

    // Offset tileId if using 16x16 tiles.
    uint16_t tileId = tile.data.tileId;
    if (xOff >= 8)
    {
        tileId += 1;
        xOff &= 0x07;
    }
    if (yOff >= 8)
    {
        tileId += 0x10;
        yOff &= 0x07;
    }

    // From here on, tiles are always 8x8.

    uint16_t addr = bgChrAddr[bg] + (tileId * 8 * bpp);
    uint8_t pixelVal = GetTilePixelData(addr, xOff, yOff, bpp);

    info.colorId = pixelVal;
    info.color = GetColorValueFromPalette(bg, tile.data.paletteId, pixelVal);
    info.isTransparent = (pixelVal == 0);
    info.bg = bg;
    return info;
}


Ppu::PixelInfo Ppu::GetBgPixelInfoMode7(uint16_t screenX, uint16_t screenY)
{
    PixelInfo info;

    auto Clip = [](int value)
    {
        if (value & 0x2000)
            return value | ~0x03FF;
        return value & 0x03FF;
    };

    int clippedX = Clip(m7HOffset - m7x);
    int clippedY = Clip(m7VOffset - m7y);

    int x = ((m7a * clippedX) & ~0x3F) +
            ((m7b * clippedY) & ~0x3F) +
            ((m7b * screenY) & ~0x3F) +
            (m7x << 8);

    int y = ((m7c * clippedX) & ~0x3F) +
            ((m7d * clippedY) & ~0x3F) +
            ((m7d * screenY) & ~0x3F) +
            (m7y << 8);

    // TODO: Move the above code to only execute once per scanline.

    info.bg = eBG1;
    info.isOnMainScreen = mainScreenLayerEnabled[eBG1];
    info.isOnSubScreen = subScreenLayerEnabled[eBG1];

    int realX = (x + (m7a * screenX)) >> 8;
    int realY = (y + (m7c * screenX)) >> 8;
    int xOff = realX & 0x07;
    int yOff = realY & 0x07;

    if (m7ExtendedFill && ((realX & ~0x3FF) || (realY & ~0x3FF)))
    {
        if (m7FillColorTile0)
        {
            info.colorId = vram[(((yOff << 3) + xOff) << 1) + 1];
            info.color = GetColorValueFromPalette(eBG1, 0, info.colorId);
            info.isTransparent = (info.colorId == 0);
        }
        else
        {
            info.colorId = 0;
            info.color = GetColorValueFromPalette(eBG1, 0, 0);
            info.isTransparent = true;
        }

        return info;
    }

    realX &= 0x3FF;
    realY &= 0x3FF;

    uint16_t tileIdAddr = (((realY & ~0x07) << 4) + (realX >> 3)) << 1;
    uint8_t tileId = vram[tileIdAddr];

    uint16_t colorAddr = (((tileId << 6) + (yOff << 3) + xOff) << 1) + 1;
    info.colorId = vram[colorAddr];
    info.color = GetColorValueFromPalette(eBG1, 0, info.colorId);
    info.isTransparent = (info.colorId == 0);

    return info;
}


uint8_t Ppu::GetSpritesOnScanline(uint8_t scanline, std::array<Sprite, 32> &sprites)
{
    uint8_t count = 0;
    uint8_t tileCount = 0;

    // TODO: Handle sprite priority rotation.
    for (int i = 0; i < 128; i++)
    {
        uint16_t spriteOffset = i * 4;
        uint16_t spriteOffsetExt = 512 + (i / 4);
        uint8_t spriteDataExt = (oam[spriteOffsetExt] >> ((i & 0x03) * 2)) & 0x03;

        uint8_t spriteHeight = OBJ_V_SIZE_LOOKUP[objSize][spriteDataExt >> 1];
        uint8_t spriteY = oam[spriteOffset + 1];
        if (scanline < spriteY || scanline > (spriteY + spriteHeight - 1))
            continue;

        uint8_t signExtend[2] = {0, 0xFF};
        uint8_t spriteWidth = OBJ_H_SIZE_LOOKUP[objSize][spriteDataExt >> 1];
        // if the high bit of x is set, extend the negative sign across the entire top byte.
        int16_t spriteX = static_cast<int16_t>(Bytes::Make16Bit(signExtend[spriteDataExt & 0x01], oam[spriteOffset]));
        // Sprites with x == -256 still count due to a bug in the PPU.
        if (spriteX != -256 && (spriteX + spriteWidth - 1) < 0)
            continue;

        if (count < 32)
        {
            sprites[count].xPos = spriteX;
            sprites[count].yPos = spriteY;
            sprites[count].tileId = oam[spriteOffset + 2];
            sprites[count].isUpperTable = oam[spriteOffset + 3] & 0x01;
            sprites[count].paletteId = (oam[spriteOffset + 3] >> 1) & 0x07;
            sprites[count].priority = (oam[spriteOffset + 3] >> 4) & 0x03;
            sprites[count].flipX = Bytes::GetBit<6>(oam[spriteOffset + 3]);
            sprites[count].flipY = Bytes::GetBit<7>(oam[spriteOffset + 3]);
            sprites[count].width = spriteWidth;
            sprites[count].height = spriteHeight;
            count++;

            // For now, just report that there are too many tiles per scanline. We'll still draw them.
            // TODO: Don't draw these.
            tileCount += spriteWidth / 8;
            if (tileCount > 34)
                Bytes::SetBit<7>(regSTAT77);
        }
        else
        {
            // If we got here it means there are more than 32 sprites on the scanline.
            Bytes::SetBit<6>(regSTAT77);
            break;
        }
    }

    if (tileCount > 34)
        LogDebug("%d tiles on scanline %d", tileCount, scanline);

    return count;
}


Ppu::PixelInfo Ppu::GetSpritePixelInfo(uint16_t screenX, uint16_t screenY, const std::array<Ppu::Sprite, 32> &sprites, uint8_t spriteCount)
{
    PixelInfo info;

    // Check if layer is disabled in emulator GUI.
    if (!enableLayer[eOBJ])
        return info;

    info.isOnMainScreen = mainScreenLayerEnabled[eOBJ];
    info.isOnSubScreen = subScreenLayerEnabled[eOBJ];

    // Check if the layer is enabled for at least one screen.
    if (!info.isOnMainScreen && !info.isOnSubScreen)
        return info;

    WindowInfo window = GetBgWindowValue(eOBJ, screenX);

    // Disable the pixel for the screen if it's inside the window and windows are enabled for the bg.
    info.isOnMainScreen &= !(window.isInside && window.isOnMainScreen);
    info.isOnSubScreen &= !(window.isInside && window.isOnSubScreen);

    // If all screens are inside the window, the pixel is never drawn.
    if (!info.isOnMainScreen && !info.isOnSubScreen)
        return info;

    for (int i = 0; i < spriteCount; i++)
    {
        const Sprite &cur = sprites[i];

        if (screenX < cur.xPos || screenX > (cur.xPos + cur.width - 1))
            continue;

        uint8_t tileX = (screenX - cur.xPos) / 8;
        uint8_t tileY = (screenY - cur.yPos) / 8;
        uint8_t xOff = (screenX - cur.xPos) & 0x07;
        uint8_t yOff = (screenY - cur.yPos) & 0x07;

        if (cur.flipX)
        {
            uint8_t tilesPerX = cur.width / 8;
            tileX = (tilesPerX - 1) - tileX;
            xOff = 7 - xOff;
        }
        if (cur.flipY)
        {
            uint8_t tilesPerY = cur.height / 8;
            tileY = (tilesPerY - 1) - tileY;
            yOff = 7 - yOff;
        }

        // The sprite table is a 16x16 table. cur.tileId is the top left tile of the sprite, so add the tileX/tileY offsets to
        // get the tile that contains the pixel we want. The tileId is stored as rrrrcccc where rrrr is the row and cccc is the column.
        // Rows and columns above F wrap to 0.
        uint8_t tileId = (((cur.tileId >> 4) + tileY) << 4) | ((cur.tileId + tileX) & 0x0F);
        uint16_t tileAddr = objBaseAddr[cur.isUpperTable] + (tileId * 8 * OBJ_BPP);

        uint8_t pixelVal = GetTilePixelData(tileAddr, xOff, yOff, OBJ_BPP);
        if (pixelVal != 0)
        {
            info.paletteId = cur.paletteId;
            info.colorId = pixelVal;
            info.bg = eOBJ;
            info.priority = cur.priority;
            info.color = GetColorValueFromPalette(eOBJ, info.paletteId, info.colorId);
            info.isTransparent = (pixelVal == 0);
            break;
        }
    }

    return info;
}


template <Ppu::EScreenType Screen>
Ppu::PixelInfo Ppu::GetPixelInfo(uint16_t screenX, uint16_t screenY, const std::array<Ppu::Sprite, 32> &sprites, uint8_t spriteCount)
{
    PixelInfo bgInfo[4];

    const uint16_t spriteX = IsHiRes() ? screenX / 2 : screenX;
    PixelInfo spriteInfo = GetSpritePixelInfo(spriteX, screenY, sprites, spriteCount);

    switch (bgMode)
    {
        case 0:
            // Sprites with priority 3
            if (spriteInfo.priority == 3 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 1
            bgInfo[eBG1] = GetBgPixelInfo(eBG1, screenX, screenY);
            if (bgInfo[eBG1].priority && bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // BG2 tiles with priority 1
            bgInfo[eBG2] = GetBgPixelInfo(eBG2, screenX, screenY);
            if (bgInfo[eBG2].priority && bgInfo[eBG2].IsNotTransparent<Screen>())
                return bgInfo[eBG2];
            // Sprites with priority 2
            if (spriteInfo.priority == 2 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 0
            if (bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // BG2 tiles with priority 0
            if (bgInfo[eBG2].IsNotTransparent<Screen>())
                return bgInfo[eBG2];
            // Sprites with priority 1
            if (spriteInfo.priority == 1 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG3 tiles with priority 1
            bgInfo[eBG3] = GetBgPixelInfo(eBG3, screenX, screenY);
            if (bgInfo[eBG3].priority && bgInfo[eBG3].IsNotTransparent<Screen>())
                return bgInfo[eBG3];
            // BG4 tiles with priority 1
            bgInfo[eBG4] = GetBgPixelInfo(eBG4, screenX, screenY);
            if (bgInfo[eBG4].priority && bgInfo[eBG4].IsNotTransparent<Screen>())
                return bgInfo[eBG4];
            // Sprites with priority 0
            if (spriteInfo.priority == 0 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG3 tiles with priority 0
            if (bgInfo[eBG3].IsNotTransparent<Screen>())
                return bgInfo[eBG3];
            // BG4 tiles with priority 0
            if (bgInfo[eBG4].IsNotTransparent<Screen>())
                return bgInfo[eBG4];
            break;
        case 1:
            // BG3 tiles with priority 1 if bit 3 of regBGMODE is set
            if (bgMode1Bg3Priority)
            {
                bgInfo[eBG3] = GetBgPixelInfo(eBG3, screenX, screenY);
                if (bgInfo[eBG3].priority && bgInfo[eBG3].IsNotTransparent<Screen>())
                    return bgInfo[eBG3];
            }
            // Sprites with priority 3
            if (spriteInfo.priority == 3 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 1
            bgInfo[eBG1] = GetBgPixelInfo(eBG1, screenX, screenY);
            if (bgInfo[eBG1].priority && bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // BG2 tiles with priority 1
            bgInfo[eBG2] = GetBgPixelInfo(eBG2, screenX, screenY);
            if (bgInfo[eBG2].priority && bgInfo[eBG2].IsNotTransparent<Screen>())
                return bgInfo[eBG2];
            // Sprites with priority 2
            if (spriteInfo.priority == 2 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 0
            if (bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // BG2 tiles with priority 0
            if (bgInfo[eBG2].IsNotTransparent<Screen>())
                return bgInfo[eBG2];
            // Sprites with priority 1
            if (spriteInfo.priority == 1 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // Skip loading if we already loaded it above.
            if (!bgMode1Bg3Priority)
                bgInfo[eBG3] = GetBgPixelInfo(eBG3, screenX, screenY);
            // BG3 tiles with priority 1 if bit 3 of regBGMODE is clear
            if (bgInfo[eBG3].priority && bgInfo[eBG3].IsNotTransparent<Screen>())
                return bgInfo[eBG3];
            // Sprites with priority 0
            if (spriteInfo.priority == 0 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG3 tiles with priority 0
            if (bgInfo[eBG3].IsNotTransparent<Screen>())
                return bgInfo[eBG3];
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            // Sprites with priority 3
            if (spriteInfo.priority == 3 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 1
            bgInfo[eBG1] = GetBgPixelInfo(eBG1, screenX, screenY);
            if (bgInfo[eBG1].priority && bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // Sprites with priority 2
            if (spriteInfo.priority == 2 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG2 tiles with priority 1
            bgInfo[eBG2] = GetBgPixelInfo(eBG2, screenX, screenY);
            if (bgInfo[eBG2].priority && bgInfo[eBG2].IsNotTransparent<Screen>())
                return bgInfo[eBG2];
            // Sprites with priority 1
            if (spriteInfo.priority == 1 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 0
            if (bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // Sprites with priority 0
            if (spriteInfo.priority == 0 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG2 tiles with priority 0
            if (bgInfo[eBG2].IsNotTransparent<Screen>())
                return bgInfo[eBG2];
            break;
        case 6:
            // Sprites with priority 3
            if (spriteInfo.priority == 3 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 1
            bgInfo[eBG1] = GetBgPixelInfo(eBG1, screenX, screenY);
            if (bgInfo[eBG1].priority && bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // Sprites with priority 2
            if (spriteInfo.priority == 2 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // Sprites with priority 1
            if (spriteInfo.priority == 1 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1 tiles with priority 0
            if (bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // Sprites with priority 0
            if (spriteInfo.priority == 0 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            break;
        case 7:
            // Sprites with priority 3
            if (spriteInfo.priority == 3 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // Sprites with priority 2
            if (spriteInfo.priority == 2 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // Sprites with priority 1
            if (spriteInfo.priority == 1 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            // BG1
            bgInfo[eBG1] = GetBgPixelInfoMode7(screenX, screenY);
            if (bgInfo[eBG1].IsNotTransparent<Screen>())
                return bgInfo[eBG1];
            // Sprites with priority 0
            if (spriteInfo.priority == 0 && spriteInfo.IsNotTransparent<Screen>())
                return spriteInfo;
            break;
    }

    // Nothing drew to this pixel.
    return PixelInfo();
}


uint16_t Ppu::GetColorValueFromPalette(EBgLayer bg, uint8_t paletteId, uint8_t colorId)
{
    uint16_t paletteOffset = 0;
    if (colorId != 0)
    {
        if (bg == eOBJ)
            paletteOffset = (paletteId << OBJ_BPP) + 128;
        else if (bgMode == 0)
            paletteOffset = (paletteId << BG_BPP_LOOKUP[bgMode][bg]) + (bg * 0x20);
        else
            paletteOffset = (paletteId << BG_BPP_LOOKUP[bgMode][bg]);
    }

    paletteOffset = (paletteOffset + colorId) << 1;

    uint16_t color = Bytes::Make16Bit(cgram[paletteOffset + 1], cgram[paletteOffset]);
    return color;
}


uint32_t Ppu::PerformColorMath(uint16_t mainColor, bool colorClipped, uint16_t screenX, uint16_t screenY, const std::array<Ppu::Sprite, 32> &sprites, uint8_t spriteCount)
{
    Bgr555 subColor;
    bool halve = halfColorMath && !colorClipped;

    if (colAddend)
    {
        PixelInfo subScreenPixel = GetPixelInfo<EScreenType::SubScreen>(screenX, screenY, sprites, spriteCount);
        if (subScreenPixel.IsNotTransparent<EScreenType::SubScreen>())
        {
            subColor = subScreenPixel.color;
        }
        else
        {
            // Transparent, use default fixed color value for subColor and disable half math.
            subColor = fixedColor;
            halve = false;
        }
    }
    else
    {
        subColor = fixedColor;
    }

    Bgr555 newColor(mainColor);

    if (colorSubtract)
    {
        newColor.Subtract(subColor, halve);
    }
    else
    {
        newColor.Add(subColor, halve);
    }

    return newColor.ToARGB888(brightness);
}


void Ppu::DrawScanline(uint8_t scanline)
{
    if (isForcedBlank)
    {
        for (int i = 0; i < SCREEN_X; i++)
        {
            uint32_t pixelOffset = ((scanline * 2) * SCREEN_X) + i;
            frameBuffer[pixelOffset] = 0;
            frameBuffer[pixelOffset + SCREEN_X] = 0;
        }
        return;
    }

    if (windowChanged)
        GenerateWindowBitmaps();

    std::array<Sprite, 32> sprites;
    int spriteCount = GetSpritesOnScanline(scanline, sprites);

    const int screenWidth = IsHiRes() ? SCREEN_X : SCREEN_X / 2;
    for (int x = 0; x < screenWidth; x++)
    {
        PixelInfo pixel = GetPixelInfo<EScreenType::MainScreen>(x, scanline, sprites, spriteCount);

        uint32_t color;
        uint16_t mainColor = pixel.color;
        bool isInside = IsPointInsideWindow(eCOL, x);
        bool isClipped = false;

        // Clip to black depending on the color window.
        if ((clipToBlack == EColorRegion::Outside && !isInside) ||
            (clipToBlack == EColorRegion::Inside && isInside) ||
             clipToBlack == EColorRegion::Always)
        {
            mainColor = 0;
            isClipped = true;
        }

        if ((preventColorMath == EColorRegion::Outside && !isInside) ||
            (preventColorMath == EColorRegion::Inside && isInside) ||
             preventColorMath == EColorRegion::Always)
        {
            color = Bgr555(mainColor).ToARGB888(brightness);
        }
        else if (bgColorMathEnable[pixel.bg] && (pixel.bg != eOBJ || (pixel.bg == eOBJ && pixel.paletteId > 3)))
        {
            color = PerformColorMath(mainColor, isClipped, x, scanline, sprites, spriteCount);
        }
        else
        {
            color = Bgr555(mainColor).ToARGB888(brightness);
        }

        if (!IsHiRes())
        {
            const uint32_t pixelOffset = ((scanline * 2) * SCREEN_X) + (x * 2);
            frameBuffer[pixelOffset] = color;
            frameBuffer[pixelOffset + 1] = color;
            frameBuffer[pixelOffset + SCREEN_X] = color;
            frameBuffer[pixelOffset + SCREEN_X + 1] = color;
        }
        else
        {
            const uint32_t pixelOffset = ((scanline * 2) * SCREEN_X) + x;
            frameBuffer[pixelOffset] = color;
            frameBuffer[pixelOffset + SCREEN_X] = color;
        }
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


void Ppu::SetBgHOffsetWriteTwice(EBgLayer bg, uint8_t byte)
{
    LogPpu("bgOffsetLatch=%02X bgHOffsetLatch=%02X", bgOffsetLatch, bgHOffsetLatch);
    // This is a write twice register.
    bgHOffset[bg] = ((byte << 8) | (bgOffsetLatch & ~0x07) | (bgHOffsetLatch & 0x07)) & 0x3FF;
    bgOffsetLatch = byte;
    bgHOffsetLatch = byte;
    LogPpu("bgHOffset[%d]=%04X", bg, bgHOffset[bg]);
}


void Ppu::SetBgVOffsetWriteTwice(EBgLayer bg, uint8_t byte)
{
    LogPpu("bgOffsetLatch=%02X", bgOffsetLatch);
    // This is a write twice register.
    bgVOffset[bg] = ((byte << 8) | bgOffsetLatch) & 0x3FF;
    bgOffsetLatch = byte;
    LogPpu("bgVOffset[%d]=%04X", bg, bgVOffset[bg]);
}


uint16_t Ppu::TranslateVramAddress(uint16_t addr, uint8_t translate)
{
    switch (translate)
    {
        case 1:
            return (addr & 0xFF00) | ((addr & 0xE0) >> 5) | ((addr & 0x1F) << 3);
        case 2:
            return (addr & 0xFE00) | ((addr & 0x1C0) >> 6) | ((addr & 0x3F) << 3);
        case 3:
            return (addr & 0xFC00) | ((addr & 0x380) >> 7) | ((addr & 0x7F) << 3);
        default:
            return addr;
    }
}


void Ppu::M7Multiply()
{
    int32_t result = static_cast<int16_t>(m7a) * static_cast<int8_t>(regM7B);
    regMPYH = Bytes::GetByte<2>(result);
    regMPYM = Bytes::GetByte<1>(result);
    regMPYL = Bytes::GetByte<0>(result);
}