#pragma once

#include "Zlsnes.h"
#include "DisplayInterface.h"
#include "IoRegisterProxy.h"
#include "TimerObserver.h"

class DebuggerInterface;
class Memory;
class Timer;

const size_t OAM_SIZE = 544;
const size_t VRAM_SIZE = 0x10000;
const size_t CGRAM_SIZE = 512;


enum EBgLayer
{
    eBG1,
    eBG2,
    eBG3,
    eBG4,
    eOBJ
};


class Ppu : public IoRegisterProxy, public HBlankObserver, public VBlankObserver
{
public:
    Ppu(Memory *memory, Timer *timer, DisplayInterface *displayInterface, DebuggerInterface *debuggerInterface = nullptr);
    virtual ~Ppu() {}

    // Inherited from IoRegisterProxy.
    uint8_t ReadRegister(EIORegisters ioReg) const override;
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;

    // Used for debugging.
    uint8_t *GetOamPtr() {return &oam[0];}
    uint8_t *GetVramPtr() {return &vram[0];}
    uint8_t *GetCgramPtr() {return &cgram[0];}

protected:
    // Inherited from HBlankObserver.
    void ProcessHBlankStart(uint32_t scanline) override;
    void ProcessHBlankEnd(uint32_t scanline) override;
    // Inherited from VBlankObserver.
    void ProcessVBlankStart() override;
    void ProcessVBlankEnd() override;

private:
    struct PixelInfo
    {
        uint8_t paletteId;
        uint8_t colorId;
        uint8_t bg;
        uint8_t priority;
        PixelInfo() : paletteId(0), colorId(0), bg(0), priority(0) {}
    };

    struct Sprite
    {
        int16_t xPos;
        uint8_t yPos;
        uint8_t tileId;
        bool isUpperTable;
        uint8_t paletteId;
        uint8_t priority;
        bool flipX;
        bool flipY;
        uint8_t width;
        uint8_t height;
        Sprite() : xPos(0), yPos(0), tileId(0), isUpperTable(false), paletteId(0), priority(0), flipX(0), flipY(0), width(0), height(0) {}
    };

    uint32_t ConvertBGR555toARGB888(uint16_t bgrColor);
    void AdjustBrightness(uint8_t brightness);

    uint8_t GetTilePixelData(uint16_t addr, uint8_t xOff, uint8_t yOff, uint8_t bpp) const;

    uint16_t GetBgTilemapEntry(EBgLayer bg, uint16_t tileX, uint16_t tileY) const;
    PixelInfo GetBgPixelInfo(EBgLayer bg, uint16_t screenX, uint16_t screenY);

    uint8_t GetSpritesOnScanline(uint8_t scanline, std::array<Sprite, 32> &sprites);
    PixelInfo GetSpritePixelInfo(uint16_t screenX, uint16_t screenY, std::array<Ppu::Sprite, 32> &sprites, uint8_t spriteCount);

    PixelInfo GetPixelInfo(uint16_t screenX, uint16_t screenY, std::array<Ppu::Sprite, 32> &sprites, uint8_t spriteCount);

    void DrawScanline(uint8_t scanline);
    void DrawScreen();
    void DrawFullScreen(); // Used when debugging to update the screen.

    void SetBgHOffsetWriteTwice(EBgLayer bg, uint8_t byte);
    void SetBgVOffsetWriteTwice(EBgLayer bg, uint8_t byte);

    uint16_t TranslateVramAddress(uint16_t addr, uint8_t translate);

    std::array<uint8_t, OAM_SIZE> oam;
    std::array<uint8_t, VRAM_SIZE> vram;
    std::array<uint8_t, CGRAM_SIZE> cgram;
    std::array<uint32_t, CGRAM_SIZE / 2> palette;
    std::array<uint32_t, SCREEN_X * SCREEN_Y> frameBuffer;

    Memory *memory;
    Timer *timer;
    DebuggerInterface *debuggerInterface;
    DisplayInterface *displayInterface;

    bool isHBlank;
    bool isVBlank;
    uint32_t scanline;

    // INIDISP - 0x2100
    bool isForcedBlank;
    uint8_t brightness;

    // OBJSEL - 0x2101
    uint8_t objSize;
    uint16_t objBaseAddr[2];

    // OAMADDL/OAMADDH - 0x2102/0x2103
    uint16_t oamRwAddr;
    uint8_t oamLatch;
    bool objPriorityRotation;

    // BGMODE - 0x2105
    uint8_t bgMode;
    bool bgMode1Bg3Priority;
    uint8_t bgChrSize[4];

    // BGxSC - 0x2107-0x210A
    uint16_t bgTilemapAddr[4];
    uint8_t bgTilemapWidth[4];
    uint8_t bgTilemapHeight[4];

    // BGxyNBA - 0x210B-0x210C
    uint16_t bgChrAddr[4];

    // BGxHOFS/BGxVOFS - 0x210D-0x2114
    uint8_t bgOffsetLatch;
    uint8_t bgHOffsetLatch;
    uint16_t bgHOffset[4];
    uint16_t bgVOffset[4];

    // VMAIN - 0x2115
    uint8_t vramIncrement;
    bool isVramIncrementOnHigh;
    uint8_t vramAddrTranslation;

    // VMADDL/VMADDH - 0x2116/0x2117
    uint16_t vramRwAddr;
    uint8_t vramPrefetch[2];

    // CGADD - 0x2121
    uint16_t cgramRwAddr;
    uint8_t cgramLatch;

    // TM/TS - 0x212c/0x212D
    bool mainScreenLayers[5];
    bool subScreenLayers[5];

    //Write-only
    uint8_t &regINIDISP; // 0x2100 Display Control 1
    uint8_t &regOBJSEL;  // 0x2101 Object Size and Object Base
    uint8_t &regOAMADDL; // 0x2102 OAM Address (lower 8bit)
    uint8_t &regOAMADDH; // 0x2103 OAM Address (upper 1bit) and Priority Rotation
    uint8_t &regOAMDATA; // 0x2104 OAM Data Write (write-twice)
    uint8_t &regBGMODE;  // 0x2105 BG Mode and BG Character Size
    uint8_t &regMOSAIC;  // 0x2106 Mosaic Size and Mosaic Enable
    uint8_t &regBG1SC;   // 0x2107 BG1 Screen Base and Screen Size
    uint8_t &regBG2SC;   // 0x2108 BG2 Screen Base and Screen Size
    uint8_t &regBG3SC;   // 0x2109 BG3 Screen Base and Screen Size
    uint8_t &regBG4SC;   // 0x210A BG4 Screen Base and Screen Size
    uint8_t &regBG12NBA; // 0x210B BG Character Data Area Designation
    uint8_t &regBG34NBA; // 0x210C BG Character Data Area Designation
    uint8_t &regBG1HOFS; // 0x210D BG1 Horizontal Scroll (X) (write-twice) / M7HOFS
    uint8_t &regBG1VOFS; // 0x210E BG1 Vertical Scroll (Y)   (write-twice) / M7VOFS
    uint8_t &regBG2HOFS; // 0x210F BG2 Horizontal Scroll (X) (write-twice)
    uint8_t &regBG2VOFS; // 0x2110 BG2 Vertical Scroll (Y)   (write-twice)
    uint8_t &regBG3HOFS; // 0x2111 BG3 Horizontal Scroll (X) (write-twice)
    uint8_t &regBG3VOFS; // 0x2112 BG3 Vertical Scroll (Y)   (write-twice)
    uint8_t &regBG4HOFS; // 0x2113 BG4 Horizontal Scroll (X) (write-twice)
    uint8_t &regBG4VOFS; // 0x2114 BG4 Vertical Scroll (Y)   (write-twice)
    uint8_t &regVMAIN;   // 0x2115 VRAM Address Increment Mode
    uint8_t &regVMADDL;  // 0x2116 VRAM Address (lower 8bit)
    uint8_t &regVMADDH;  // 0x2117 VRAM Address (upper 8bit)
    uint8_t &regVMDATAL; // 0x2118 VRAM Data Write (lower 8bit)
    uint8_t &regVMDATAH; // 0x2119 VRAM Data Write (upper 8bit)
    uint8_t &regM7SEL;   // 0x211A Rotation/Scaling Mode Settings
    uint8_t &regM7A;     // 0x211B Rotation/Scaling Parameter A & Maths 16bit operand (write-twice)
    uint8_t &regM7B;     // 0x211C Rotation/Scaling Parameter B & Maths 8bit operand (write-twice)
    uint8_t &regM7C;     // 0x211D Rotation/Scaling Parameter C         (write-twice)
    uint8_t &regM7D;     // 0x211E Rotation/Scaling Parameter D         (write-twice)
    uint8_t &regM7X;     // 0x211F Rotation/Scaling Center Coordinate X (write-twice)
    uint8_t &regM7Y;     // 0x2120 Rotation/Scaling Center Coordinate Y (write-twice)
    uint8_t &regCGADD;   // 0x2121 Palette CGRAM Address
    uint8_t &regCGDATA;  // 0x2122 Palette CGRAM Data Write             (write-twice)
    uint8_t &regW12SEL;  // 0x2123 Window BG1/BG2 Mask Settings
    uint8_t &regW34SEL;  // 0x2124 Window BG3/BG4 Mask Settings
    uint8_t &regWOBJSEL; // 0x2125 Window OBJ/MATH Mask Settings
    uint8_t &regWH0;     // 0x2126 Window 1 Left Position (X1)
    uint8_t &regWH1;     // 0x2127 Window 1 Right Position (X2)
    uint8_t &regWH2;     // 0x2128 Window 2 Left Position (X1)
    uint8_t &regWH3;     // 0x2129 Window 2 Right Position (X2)
    uint8_t &regWBGLOG;  // 0x212A Window 1/2 Mask Logic (BG1-BG4)
    uint8_t &regWOBJLOG; // 0x212B Window 1/2 Mask Logic (OBJ/MATH)
    uint8_t &regTM;      // 0x212C Main Screen Designation
    uint8_t &regTS;      // 0x212D Sub Screen Designation
    uint8_t &regTMW;     // 0x212E Window Area Main Screen Disable
    uint8_t &regTSW;     // 0x212F Window Area Sub Screen Disable
    uint8_t &regCGWSEL;  // 0x2130 Color Math Control Register A
    uint8_t &regCGADSUB; // 0x2131 Color Math Control Register B
    uint8_t &regCOLDATA; // 0x2132 Color Math Sub Screen Backdrop Color
    uint8_t &regSETINI;  // 0x2133 Display Control 2

    // Read-only
    uint8_t &regMPYL;    // 0x2134 PPU1 Signed Multiply Result   (lower 8bit)
    uint8_t &regMPYM;    // 0x2135 PPU1 Signed Multiply Result   (middle 8bit)
    uint8_t &regMPYH;    // 0x2136 PPU1 Signed Multiply Result   (upper 8bit)
    uint8_t &regSLHV;    // 0x2137 PPU1 Latch H/V-Counter by Software (Read=Strobe)
    uint8_t &regRDOAM;   // 0x2138 PPU1 OAM Data Read            (read-twice)
    uint8_t &regRDVRAML; // 0x2139 PPU1 VRAM Data Read           (lower 8bits)
    uint8_t &regRDVRAMH; // 0x213A PPU1 VRAM Data Read           (upper 8bits)
    uint8_t &regRDCGRAM; // 0x213B PPU2 CGRAM Data Read (Palette)(read-twice)
    uint8_t &regOPHCT;   // 0x213C PPU2 Horizontal Counter Latch (read-twice)
    uint8_t &regOPVCT;   // 0x213D PPU2 Vertical Counter Latch   (read-twice)
    uint8_t &regSTAT77;  // 0x213E PPU1 Status and PPU1 Version Number
    uint8_t &regSTAT78;  // 0x213F PPU2 Status and PPU2 Version Number

    friend class PpuTest;
    friend class InfoWindow;
    friend class DebuggerWindow;
};